#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <SDL2/SDL.h>
#include <opencv2/opencv.hpp>
#include "include/json/include/nlohmann/json.hpp"
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <cmath>
#include <unordered_set>

struct Ball 
{
    float x, y;
    float vx, vy;
    int radius;
    float collision_elasticity_factor; // 0 (all energy lost) to 1 (no energy loss)
    std::vector<int> color;
};

void to_json(nlohmann::json& j, const Ball& b);
void from_json(const nlohmann::json& j, Ball& b);

const std::string JSON_METADATA_FILE_NAME = "simulator_data.json";

const std::vector<std::vector<int>> POSSIBLE_BALL_COLORS = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {204, 204, 0}, {102, 204, 0},
    {0, 204, 0}, {0, 204, 102}, {0, 204, 204}, {0, 102, 204}, {0, 0, 204}, {102, 0, 204}, {204, 0, 204}, {204, 0, 102},
    {153, 76, 0}, {153, 153, 0}, {76, 153, 0}, {0, 153, 0}, {0, 153, 76}, {0, 153, 153}, {0, 76, 153}, {0, 0, 153}, {76, 0, 153},
    {153, 0, 153}, {153, 0, 76}, {204, 102, 0}, {153, 0, 0}, {204, 0, 0}, {255, 128, 0}, {255, 255, 0}, {128, 255, 0},
    {0, 255, 128}, {0, 255, 255}, {0, 128, 255}, {127, 0, 255}, {255, 0, 255}, {255, 0, 127}, {255, 51, 51}, {255, 153, 51},
    {255, 255, 51}, {153, 255, 51}, {51, 255, 51}, {51, 255, 153}, {51, 255, 255}, {51, 153, 255}, {51, 51, 255}, {153, 51, 255},
    {255, 51, 255}, {255, 51, 153}, {255, 178, 102}, {255, 255, 102}, {178, 255, 102}, {102, 255, 102}, {102, 255, 178},
    {102, 255, 255}, {102, 178, 255}, {102, 102, 255}, {178, 102, 255}, {255, 102, 255}, {255, 102, 178}};

class Simulator
{
private:
    int _window_width, _window_height;
    float _x_gravity, _y_gravity;
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    std::vector<Ball> _balls;
    std::multimap<float, std::string> _current_1D_Ball_locations;
    std::vector<std::pair<int, int>> _current_collisions;
public:
    Simulator(int width, int height);
    Simulator(int width, int height, float x_gravity, float y_gravity, std::vector<Ball>& balls);
    Simulator();
    ~Simulator();
    void runSimulation(int num_frames);
    void createVideoFromFrames(int frame_rate, const std::string& save_directory, bool remove_metadata);
    void saveSimulationMetadata() const;
    void deleteTempImageFiles();
private:
    void initializeSimulation();
    void initializeSDL();
    void createSDLWindow();
    void createSDLRenderer();
    void updateSimulation();
    void renderSimulation();
    void update1DBallLocations();
    void generateCollisionPairs();
    void handleBallCollisions();
    void handleSingleBallCollisionInstance(int ball_num1, int ball_num2, bool loseEnergy);
    void handleWallCollisionForSpecificBall(Ball& current_ball);
    std::pair<int, int> getCollidedPair();
    void saveFrame(const std::string& filename);
    void drawBall(int centerX, int centerY, int radius, std::vector<int>& color);
    void updateBallPosition(Ball& current_ball);
    bool collisionDetected(int ball_num1, int ball_num2);
    int getLastSavedPhotoFrameNum();
    void drawAllBalls();
    void loadSimulationMetadata();
};

#endif