#include "Simulator.h"

Simulator::Simulator(int width, int height)
{
    _window_width = width;
    _window_height = height;

    _x_gravity = 0;
    _y_gravity = 1;

    int rand_color_index = rand() % POSSIBLE_BALL_COLORS.size();

    Ball default_ball = {100, 100, 70, 44, 25, 1, POSSIBLE_BALL_COLORS[rand_color_index]};
    _balls.push_back(default_ball);

    initializeSimulation();
    deleteTempImageFiles();
}

Simulator::Simulator(int width, int height, float x_gravity, float y_gravity, std::vector<Ball>& balls)
{
    _window_width = width;
    _window_height = height;

    _x_gravity = x_gravity;
    _y_gravity = y_gravity;

    _balls = balls;

    initializeSimulation();
    deleteTempImageFiles();
}

Simulator::Simulator()
{
    // Loading last captured state from previous execution run
    loadSimulationMetadata();
    initializeSimulation();
}

Simulator::~Simulator() 
{
    if (_renderer) 
    {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }

    if (_window) 
    {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    SDL_Quit();
}

void Simulator::initializeSimulation()
{
    initializeSDL();
    createSDLWindow();
    createSDLRenderer();
}

void Simulator::initializeSDL()
{
    // Initialize and check to make sure SDL has been properly initialized
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
}

void Simulator::createSDLWindow()
{
    _window = SDL_CreateWindow("Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _window_width, _window_height, SDL_WINDOW_SHOWN);

    if (!_window) 
    {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    } 
}

void Simulator::createSDLRenderer()
{
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

    if (!_renderer) 
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(_window);
        SDL_Quit();
        exit(1);
    }
}

void Simulator::updateSimulation() 
{
    // General position updates for each ball
    for (Ball& ball : _balls)
        updateBallPosition(ball);

    // Handles collisions between balls
    handleBallCollisions();

    // Collisions between walls
    for (Ball& ball : _balls)
        handleWallCollisionForSpecificBall(ball);
}

void Simulator::handleWallCollisionForSpecificBall(Ball& current_ball)
{
    // Bounce off the walls
    if (current_ball.x - current_ball.radius < 0 || current_ball.x + current_ball.radius > _window_width) 
    {
        current_ball.vx = -current_ball.vx;
        
        // Adjust velocities to account for energy lost
        if (abs(current_ball.vx / current_ball.vy) < 1)
        {
            current_ball.vx *= (sqrt(current_ball.collision_elasticity_factor) + (1 - sqrt(current_ball.collision_elasticity_factor)) * (1 - abs(current_ball.vx / current_ball.vy)));
            current_ball.vy *= (sqrt(current_ball.collision_elasticity_factor) + (1 - sqrt(current_ball.collision_elasticity_factor)) * (1 - abs(current_ball.vx / current_ball.vy)));
        }
        else
        {
            current_ball.vx *= sqrt(current_ball.collision_elasticity_factor);
            current_ball.vy *= sqrt(current_ball.collision_elasticity_factor);
        }
        
        // Keep inside box bounds
        if (current_ball.x - current_ball.radius < 0)
            current_ball.x = current_ball.radius;
        else
            current_ball.x = _window_width - current_ball.radius;
    }
    
    if (current_ball.y - current_ball.radius < 0 || current_ball.y + current_ball.radius > _window_height) 
    {
        current_ball.vy = -current_ball.vy;

        if (abs(current_ball.vy / current_ball.vx) < 1)
        {
            current_ball.vx *= (sqrt(current_ball.collision_elasticity_factor) + (1 - sqrt(current_ball.collision_elasticity_factor)) * (1 - abs(current_ball.vy / current_ball.vx)));
            current_ball.vy *= (sqrt(current_ball.collision_elasticity_factor) + (1 - sqrt(current_ball.collision_elasticity_factor)) * (1 - abs(current_ball.vy / current_ball.vx)));
        }
        else 
        {
            current_ball.vx *= sqrt(current_ball.collision_elasticity_factor);
            current_ball.vy *= sqrt(current_ball.collision_elasticity_factor);
        }
        
        if (current_ball.y - current_ball.radius < 0)
            current_ball.y = current_ball.radius;
        else
            current_ball.y = _window_height - current_ball.radius;
    }
}

void Simulator::updateBallPosition(Ball& current_ball)
{
    // Update velocity
    current_ball.vx += _x_gravity;
    current_ball.vy += _y_gravity;

    // Update position
    current_ball.x += current_ball.vx;
    current_ball.y += current_ball.vy;
}

void Simulator::renderSimulation() 
{
    // Draw all the balls
    drawAllBalls();
}

void Simulator::saveFrame(const std::string& filename) 
{
    SDL_Surface* sshot = SDL_CreateRGBSurface(0, _window_width, _window_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_RenderReadPixels(_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
    cv::Mat img = cv::Mat(sshot->h, sshot->w, CV_8UC4, sshot->pixels);
    cv::cvtColor(img, img, cv::COLOR_BGRA2BGR); // Convert to BGR format
    cv::imwrite(filename, img);
    SDL_FreeSurface(sshot);
}

void Simulator::runSimulation(int num_frames)
{
    int lastSavedFrameNum = getLastSavedPhotoFrameNum();

    for (int frame = lastSavedFrameNum + 1; frame < num_frames + lastSavedFrameNum + 1; ++frame)
    {
        // State of all free-moving objects gets updated by one frame
        updateSimulation();

        // Clear screen
        SDL_SetRenderDrawColor(_renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(_renderer);

        // Update render screen based on updated object states
        renderSimulation();

        // Save the frame to an image file
        std::string filename = "Image Frames/frame_" + std::to_string(frame) + ".png";
        saveFrame(filename);
    }
}

void Simulator::deleteTempImageFiles() 
{
    try 
    {
        for (const auto& entry : std::filesystem::directory_iterator("Image Frames/")) 
        {
            if (entry.is_regular_file() && entry.path().extension() == ".png")
                std::filesystem::remove(entry.path());
        }
    } 
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        exit(1);
    }
    catch (const std::exception& e)
    {
        std::cerr << "General error: " << e.what() << std::endl;
        exit(1);
    }
}

int Simulator::getLastSavedPhotoFrameNum() 
{
    std::string directoryPath = "Image Frames/";
    int largestFrameNum = -1;

    std::regex frameRegex(R"(frame_(\d+)\.png)");
    std::smatch match;

    try 
    {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) 
        {
            if (entry.is_regular_file()) 
            {
                std::string filename = entry.path().filename().string();
                if (std::regex_match(filename, match, frameRegex)) 
                {
                    int frameNum = std::stoi(match[1].str());
                    if (frameNum > largestFrameNum)
                        largestFrameNum = frameNum;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        exit(1);
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "General error: " << e.what() << std::endl;
        exit(1);
    }

    return largestFrameNum;
}

void Simulator::createVideoFromFrames(int frame_rate, const std::string& save_directory, bool remove_metadata)
{
    std::string video_file = save_directory + "/simulation.mp4";
    cv::VideoWriter writer(video_file, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), frame_rate, cv::Size(_window_width, _window_height));
    int num_frames = getLastSavedPhotoFrameNum() + 1;

    if (!writer.isOpened()) 
    {
        std::cerr << "Could not open the output video file to write." << std::endl;
        exit(1);
    }

    for (int frame = 0; frame < num_frames; ++frame) 
    {
        std::string filename = "Image Frames/frame_" + std::to_string(frame) + ".png";
        cv::Mat img = cv::imread(filename);

        if (img.empty()) 
        {
            std::cerr << "Could not read frame: " << filename << std::endl;
            exit(1);
        }

        writer.write(img);
    }

    writer.release();

    if (remove_metadata)
        deleteTempImageFiles();
    else
        saveSimulationMetadata();

    std::cout << "Video creation completed successfully!\n";
}

void Simulator::drawAllBalls()
{
    for (Ball& ball : _balls)
        drawBall(static_cast<int>(ball.x), static_cast<int>(ball.y), ball.radius, ball.color);
}

void Simulator::drawBall(int centerX, int centerY, int radius, std::vector<int>& color) 
{
    // Set the color for drawing the current specified ball
    SDL_SetRenderDrawColor(_renderer, color[0], color[1], color[2], 0xFF);

    for (int w = 0; w < radius * 2; w++) 
    {
        for (int h = 0; h < radius * 2; h++) 
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset

            if ((dx*dx + dy*dy) <= (radius * radius)) 
            {
                SDL_RenderDrawPoint(_renderer, centerX + dx, centerY + dy);
            }
        }
    }
}

void Simulator::update1DBallLocations()
{
    _current_1D_Ball_locations.clear();

    for (int i = 0; i < _balls.size(); i++)
    {
        _current_1D_Ball_locations.insert(std::make_pair(_balls[i].x - _balls[i].radius, std::to_string(i) + "a"));
        _current_1D_Ball_locations.insert(std::make_pair(_balls[i].x + _balls[i].radius, std::to_string(i) + "b"));
    }
}

void Simulator::generateCollisionPairs()
{
    std::vector<std::pair<int, int>> possible_collision_pairs;
    std::unordered_set<int> buffer_nums;

    for (const auto& pair : _current_1D_Ball_locations) 
    {
        int ball_num = std::stoi(pair.second.substr(0, pair.second.length() - 1));

        if (pair.second.find("a") != std::string::npos) // 'a' found so corresponds to left ball side
            buffer_nums.insert(ball_num);
        else
        {
            // Add possibility of collision between ball_num and all other nums inside buffer
            for (auto it = buffer_nums.begin(); it != buffer_nums.end(); ++it) 
            {
                if (*it != ball_num)
                    possible_collision_pairs.push_back(std::make_pair(ball_num, *it));
            }

            buffer_nums.erase(ball_num);
        }
    }

    _current_collisions.clear();

    for (std::pair<int, int>& pair : possible_collision_pairs)
    {
        if (collisionDetected(pair.first, pair.second))
            _current_collisions.push_back(pair);
    }
}

std::pair<int, int> Simulator::getCollidedPair()
{
    update1DBallLocations();
    generateCollisionPairs();

    if (_current_collisions.size() > 0)
        return _current_collisions[rand() % _current_collisions.size()];

    return std::make_pair(-1, -1);
}

bool Simulator::collisionDetected(int ball_num1, int ball_num2)
{
    float distance_between_midpoints = sqrt(pow(_balls[ball_num1].x - _balls[ball_num2].x, 2) + pow(_balls[ball_num1].y - _balls[ball_num2].y, 2));
    float sum_of_radii = _balls[ball_num1].radius + _balls[ball_num2].radius;

    return sum_of_radii > distance_between_midpoints;
}

void Simulator::handleBallCollisions() 
{
    // Maintain a sorted list of all essential one-dimensional ball locations
    update1DBallLocations();

    // Get all pairs of balls that initially are collided with eachother
    generateCollisionPairs();

    // Handles all initial collisions
    for (const std::pair<int, int>& pair : _current_collisions)
        handleSingleBallCollisionInstance(pair.first, pair.second, true);

    std::pair<int, int> pair = getCollidedPair();
    int count = 0;

    // Handle potential additional collisions caused by handling of initial collisions
    while (pair.first != -1 && count < _balls.size() * _balls.size())
    {
        handleSingleBallCollisionInstance(pair.first, pair.second, false);

        pair = getCollidedPair();
        count++;
    }
}

void Simulator::handleSingleBallCollisionInstance(int ball_num1, int ball_num2, bool loseEnergy)
{
    Ball& ball1 = _balls[ball_num1];
    Ball& ball2 = _balls[ball_num2];

    // Vector between centers of the balls
    float dx = ball1.x - ball2.x;
    float dy = ball1.y - ball2.y;

    // Distance between centers of the balls
    float d_mids = sqrt(dx * dx + dy * dy);

    float overlap = 0.5 * (d_mids - (ball1.radius + ball2.radius));

    // Displace ball1 along the line of centers
    ball1.x -= (overlap * (ball1.x - ball2.x) / d_mids) * 1.25;
    ball1.y -= (overlap * (ball1.y - ball2.y) / d_mids) * 1.25;

    // Displace ball2 along the line of centers
    ball2.x += (overlap * (ball1.x - ball2.x) / d_mids) * 1.25;
    ball2.y += (overlap * (ball1.y - ball2.y) / d_mids) * 1.25;

    // Squared radii (mass proxies)
    float m1 = ball1.radius * ball1.radius;
    float m2 = ball2.radius * ball2.radius;

    // Dot product of velocities and displacement vectors
    float dot_v1 = (ball1.vx - ball2.vx) * dx + (ball1.vy - ball2.vy) * dy;
    float dot_v2 = (ball2.vx - ball1.vx) * dx + (ball2.vy - ball1.vy) * dy;

    // Scalar terms for updating velocities
    float scalar1 = (2 * m2) / (m1 + m2) * dot_v1 / (d_mids * d_mids);
    float scalar2 = (2 * m1) / (m1 + m2) * dot_v2 / (d_mids * d_mids);

    // Update velocities of ball1 and ball2
    ball1.vx -= scalar1 * dx;
    ball1.vy -= scalar1 * dy;

    ball2.vx -= scalar2 * dx;
    ball2.vy -= scalar2 * dy;

    if (loseEnergy) // Take into account energy loss component
    {
        ball1.vx *= sqrt(ball1.collision_elasticity_factor);
        ball1.vy *= sqrt(ball1.collision_elasticity_factor);

        ball2.vx *= sqrt(ball2.collision_elasticity_factor);
        ball2.vy *= sqrt(ball2.collision_elasticity_factor);
    }
}

void Simulator::saveSimulationMetadata() const 
{
    nlohmann::json j;
    j["window_width"] = _window_width;
    j["window_height"] = _window_height;
    j["x_gravity"] = _x_gravity;
    j["y_gravity"] = _y_gravity;
    j["balls"] = _balls;

    std::ofstream file(JSON_METADATA_FILE_NAME);
    if (file.is_open()) 
    {
        file << j.dump(4);
        file.close();
    } 
    else 
    {
        std::cerr << "Unable to open file for writing\n";
        exit(1);
    }
}

void Simulator::loadSimulationMetadata() 
{
    std::ifstream file(JSON_METADATA_FILE_NAME);
    if (file.is_open()) 
    {
        nlohmann::json j;
        file >> j;
        file.close();

        j.at("window_width").get_to(_window_width);
        j.at("window_height").get_to(_window_height);
        j.at("x_gravity").get_to(_x_gravity);
        j.at("y_gravity").get_to(_y_gravity);
        j.at("balls").get_to(_balls);
    } 
    else 
    {
        std::cerr << "Unable to open file for reading\n";
        exit(1);
    }
}

void to_json(nlohmann::json& j, const Ball& b) 
{
    j = nlohmann::json{
        {"x", b.x},
        {"y", b.y},
        {"vx", b.vx},
        {"vy", b.vy},
        {"radius", b.radius},
        {"collision_elasticity_factor", b.collision_elasticity_factor},
        {"color", b.color}
    };
}

void from_json(const nlohmann::json& j, Ball& b) 
{
    j.at("x").get_to(b.x);
    j.at("y").get_to(b.y);
    j.at("vx").get_to(b.vx);
    j.at("vy").get_to(b.vy);
    j.at("radius").get_to(b.radius);
    j.at("collision_elasticity_factor").get_to(b.collision_elasticity_factor);
    j.at("color").get_to(b.color);
}



