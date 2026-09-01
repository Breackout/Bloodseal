#pragma once

#include "Global.hpp"

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

struct PlayerStats
{
    int HP = 100;
    int defence = 30;
    int attackDamage = 30;  // attacco fisico
    int magicDamage = 30;

    float attackSpeed = 0.5f;
    float maxSpeed = 300.0f;
    float lifeSteal = 0.0f;
    // coolDown abilita da mettere quando faro tutte le abilita

    void lvlUp();
    int lvl = 0;
    int lvlPoint = 0;
};

struct EnemyStats
{
    int HP = 100;
    int defence = 30;
    int attackDamage = 30;  // attacco fisico
    int magicDamage = 30;

    float attackSpeed = 0.5f;
    float maxSpeed = 300.0f;
    float lifeSteal = 0.0f;
};

class Enemy
{

};

class Player
{
    public:
        PlayerStats stats;

        // da cambiare con uno sprite
        vec2D pos = { 1000.0f, 1000.0f };
        float width = 100.0f;
        float height = 100.0f;

        // usato SOLO per il rendering (coordinate schermo, relative alla camera)
        SDL_FRect playerBox = { pos.x, pos.y, width, height };

        // variabili per il movimento
        vec2D vel = { 0.0f, 0.0f };
        float acceleration = 1500.0f;
        float maxSpeed = stats.maxSpeed;
        bool moving = false;

        // variabili per il salto
        float jumpForce = -600.0f;
        bool isGround = false;

        // variabili per il dash
        int dir = 0;
        float dashSpeed = 2000.0f;
        Uint64 dashCoulDownTimer = 0.0f;
        bool isDashing = false;
        bool dashKeyWasPressed = false;
        float dashTimer = 0.0f;
        float dashCooldownTimer = 0.0f;
        float dashDuration = 0.15f;         // quanto dura il dash
        float dashCooldownDuration = 2.0f;  // quanto aspetti prima di poterlo rifare


        void Update(float dt, const bool* keys);
        void Draw(const Camera &camera);

        // Bounding box in world-space (NON relativa alla camera), usata per le collisioni
        SDL_FRect GetWorldBox() const
        {
            return SDL_FRect{ pos.x, pos.y, width, height };
        }

    private:
        void move(float dt, const bool* keys);
};
