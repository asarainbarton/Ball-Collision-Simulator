#include "Simulator.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>

struct Config 
{
    int window_width;
    int window_height;
    int min_x_vel;
    int max_x_vel;
    int min_y_vel;
    int max_y_vel;
    float x_gravity;
    float y_gravity;
    int min_radius;
    int max_radius;
    int num_frames;
    int frame_rate;
    float ball_elasticity;
    int num_balls;
};

std::vector<Ball> get_random_balls(int num_balls, Config& config);
Config loadConfig(const std::string& filename);
bool beginNewProject();
char getSaveChoice();

int main()
{
    srand(time(nullptr));
    const std::string SETTINGS_FILE_NAME = "../config.json";
    const std::string SAVE_DIRECTORY = "../";

    bool start_new_project = beginNewProject();
    Config config = loadConfig(SETTINGS_FILE_NAME);

    if (start_new_project)
    {
        std::vector<Ball> balls = get_random_balls(config.num_balls, config);
        Simulator ball_simulator(config.window_width, config.window_height, config.x_gravity, config.y_gravity, balls);



        ball_simulator.runSimulation(config.num_frames);

        char save_choice = getSaveChoice();
        bool remove_metadata = save_choice != '2' && save_choice != '3';
        
        if (save_choice == '1' || save_choice == '2') // Save video
        {
            ball_simulator.createVideoFromFrames(config.frame_rate, SAVE_DIRECTORY, remove_metadata);
        }
        else if (remove_metadata)
            ball_simulator.deleteTempImageFiles();
        else // Option '3'
            ball_simulator.saveSimulationMetadata();
    }
    else // Load existing project
    {
        Simulator ball_simulator;



        ball_simulator.runSimulation(config.num_frames);

        char save_choice = getSaveChoice();
        bool remove_metadata = save_choice != '2' && save_choice != '3';
        
        if (save_choice == '1' || save_choice == '2') // Save video
        {
            ball_simulator.createVideoFromFrames(config.frame_rate, SAVE_DIRECTORY, remove_metadata);
        }
        else if (remove_metadata)
            ball_simulator.deleteTempImageFiles();
        else // Option '3'
            ball_simulator.saveSimulationMetadata();
    }


    return 0;
}

Config loadConfig(const std::string& filename) 
{
    try
    {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Unable to open config file");

        nlohmann::json j;
        file >> j;

        Config config;
        config.window_width = j["WINDOW_WIDTH"];
        config.window_height = j["WINDOW_HEIGHT"];
        config.min_x_vel = j["MIN_X_VEL"];
        config.max_x_vel = j["MAX_X_VEL"];
        config.min_y_vel = j["MIN_Y_VEL"];
        config.max_y_vel = j["MAX_Y_VEL"];
        config.x_gravity = j["X_GRAVITY"];
        config.y_gravity = j["Y_GRAVITY"];
        config.min_radius = j["MIN_RADIUS"];
        config.max_radius = j["MAX_RADIUS"];
        config.num_frames = j["NUM_FRAMES"];
        config.frame_rate = j["FRAME_RATE"];
        config.ball_elasticity = j["BALL_ELASTICITY"];
        config.num_balls = j["NUM_BALLS"];

        return config;
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

std::vector<Ball> get_random_balls(int num_balls, Config& config)
{
    std::vector<Ball> ball_list;

    // Define range of possible starting positions
    int min_x_pos = config.max_radius;
    int max_x_pos = config.window_width - config.max_radius;
    int min_y_pos = config.max_radius;
    int max_y_pos = config.window_height - config.max_radius;

    int x_pos, y_pos;
    int x_vel, y_vel;
    int radius, rand_color_index;

    for (int i = 0; i < num_balls; i++)
    {
        x_pos = rand() % (max_x_pos - min_x_pos + 1) + min_x_pos;
        y_pos = rand() % (max_y_pos - min_y_pos + 1) + min_y_pos;

        x_vel = rand() % (config.max_x_vel - config.min_x_vel + 1) + config.min_x_vel;
        y_vel = rand() % (config.max_y_vel - config.min_y_vel + 1) + config.min_y_vel;

        radius = rand() % (config.max_radius - config.min_radius + 1) + config.min_radius;

        rand_color_index = rand() % POSSIBLE_BALL_COLORS.size();

        Ball b = {static_cast<float>(x_pos), static_cast<float>(y_pos), static_cast<float>(x_vel), static_cast<float>(y_vel),
            radius, config.ball_elasticity, POSSIBLE_BALL_COLORS[rand_color_index]};

        ball_list.push_back(b);
    }

    return ball_list;
}

bool beginNewProject()
{
    std::string choice;
    std::cout << "Enter '1' to start a new project.\n";
    std::cout << "Enter '2' to continue working on a current project.\n";
    std::getline(std::cin, choice);

    std::cout << "Rendering now in progress...\n";

    if (choice[0] == '1')
        return true;
    
    return false;
}

char getSaveChoice()
{
    std::string choice;
    std::cout << "Rendering Complete.\n";
    std::cout << "Enter '1' to save the rendered video and delete project metadata.\n";
    std::cout << "Enter '2' to save the rendered video and keep project metadata.\n";
    std::cout << "Enter '3' to only keep project metadata for later use.\n";
    std::cout << "Enter '4' to delete project metadata.\n";
    std::getline(std::cin, choice);

    return choice[0];
}
