#pragma once

#include "Render.hpp"
#include "Global.hpp"

extern Render rend;

class Camera
{
public:
    vec2D pos;
    float zoom = 0.5f;

    void Update(const vec2D& targetPos, float targetW, float targetH, float dt);
    void SetBounds(float worldW, float worldH);

    SDL_FRect GetViewport() const
    {
        return SDL_FRect{ pos.x, pos.y, float(ScreenWidth), float(ScreenHeight) };
    }

private:
    float smoothSpeed = 5.0f; // quanto velocemente la camera "raggiunge" il player
    float worldWidth = 0.0f;
    float worldHeight = 0.0f;
};

class Player
{
    public:
        vec2D pos = { 1000.0f, 1000.0f };
        SDL_FRect playerBox = { pos.x, pos.y, 100.0f, 100.0f };
        float acceleration = 25.0f;
        float jumpForce;
        vec2D vel = { 2.5f, 200.0f };
        float maxSpeed = 5.0f;

        void Update(float dt, const bool* keys);
        void Draw(const Camera &camera);

    private:
        void move(float dt, const bool* keys);
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
        SDL_Texture* map;
        Player player;
        const bool* keys;
        Camera camera;
        SDL_FRect mapRect = { 0.0f, 0.0f, 4000.0f, 4000.0f };
};
