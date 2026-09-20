#pragma once

#include "Entity.hpp"
#include "levelManager.hpp"

class Game
{
    public:
        Game();

        void Update(float dt, const bool* keys);
        void Draw();

        void run();

    private:
        bool isRunning;

        Player player;
        const bool* keys;
        Camera camera;
        levelManager lvlManager;
};
