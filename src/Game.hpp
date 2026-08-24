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
        vec2D pos = { 1000.0f, 1000.0f };
        SDL_FRect playerBox = { pos.x, pos.y, 100.0f, 100.0f };
        float acceleration = 25.0f;
        vec2D vel = { 0.0f, 0.0f };
        float maxSpeed = 5.0f;
        bool moving = false;
        // float jumpForce = 350.0f;
        // bool grounded = false;

        void Update(float dt, const bool* keys);
        void Draw(const Camera &camera);

    private:
        void move(float dt, const bool* keys);
        // void jump(const bool* keys);
        // void dash(float dt, const bool* keys);

        // int facingDir = 1; // 1 = destra, -1 = sinistra

        // bool spaceWasPressed = false;
        // bool shiftWasPressed = false;

        // bool  isDashing        = false;
        // float dashTimer        = 0.0f;
        // float dashCooldownTimer = 0.0f;
        // float dashSpeed        = 15.0f;
        // float dashDuration     = 0.15f;
        // float dashCooldown     = 1.0f;
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
        SDL_Texture* map;
        SDL_FRect mapRect{};
        Player player;
        const bool* keys;
        Camera camera;
        Polygon ground;
};
