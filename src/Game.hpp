#pragma once

#include "Render.hpp"
#include "Global.hpp"
#include "Collision.hpp"

extern Render rend;

class Camera
{
public:
    vec2D pos;
    float zoom = 0.4f;

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
        // da cambiare con uno sprite
        vec2D pos = { 1000.0f, 1000.0f };
        SDL_FRect playerBox = { pos.x, pos.y, 100.0f, 100.0f };

        // variabili per il movimento
        vec2D vel = { 0.0f, 0.0f };        float acceleration = 25.0f;
        float maxSpeed = 5.0f;
        bool moving = false;
        float jumpForce = -600.0f;
        bool isGround = false;

        void Update(float dt, const bool* keys);
        void Draw(const Camera &camera);

    private:
        void move(float dt, const bool* keys);
        void dash(float dt, const bool* keys);
};


class Game
{
    public:
        Game();
        ~Game();

        void Update(float dt, const bool* keys);
        void Draw();

        void run();

    private:
        bool isRunning;
        // le robe per la mappa andranno poi spostate dentro la struct in global.h
        SDL_Texture* map;
        SDL_FRect mapRect{};
        Polygon ground;
        Player player;
        const bool* keys;
        Camera camera;
};
