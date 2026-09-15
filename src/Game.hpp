#pragma once

#include "Render.hpp"
#include "Global.hpp"
#include "Entity.hpp"

#include <vector>

struct level
{
    Texture map;
    SDL_FRect mapRect;

    std::vector<Rectangle> rects;
    std::vector<Triangle> tris;

    void LoadLevelCollisionInfo(const char* path);

    level(const char* texture, const char* collision);
    ~level();
};


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
        Enemy e;
        const bool* keys;
        Camera camera;
        level lvl;

        void DrawDebugCollisions();
};
