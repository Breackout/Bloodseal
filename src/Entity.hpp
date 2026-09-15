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
    float armorPenPercent;
    float armorPen;

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
    float maxSpeed = 150.0f;
    float lifeSteal = 0.0f;
};

// tutti i malus che il player puo avere
struct PlayerMalus
{
    bool isPoisoned = false;
    bool isSlowned = false;
    bool isStunned = false;
    bool isBleeding = false;

    Uint64 poisenedTimer = 0.0f;
    Uint64 slownedTimer = 0.0f;
    Uint64 stunnedTimer = 0.0f;
    Uint64 bleedingTimer = 0.0f;

    Uint64 damageTickTimer = 0.0f;


};

// tutti i malus che i nemici possono avere
struct EnemyMalus
{
    bool isPoisoned = false;
    bool isSlowed = false;
    bool isStunned = false;
    bool isCursed = false;
    bool isBleeding = false;
};



class Entity
{
    public:
        vec2D pos;
        float width = 100.0f;
        float height = 100.0f;

        SDL_FRect box;

        // variabili per il movimento
        vec2D vel = { 0.0f, 0.0f };
        float acceleration = 1500.0f;
        float maxSpeed;
        bool moving = false;

        // variabili per il salto
        bool isGround = false;
        float jumpForce = -600;

        // Bounding box in world-space (NON relativa alla camera), usata per le collisioni
        SDL_FRect GetWorldBox() const
        {
            return SDL_FRect{ pos.x, pos.y, width, height };
        }

        virtual void Draw(const Camera& camera);
};

class Player : public Entity
{
    public:
        Player();

        // logica aggiungere un costruttore che aggiunga tutte le animazione
        // del player e una logiaca per gestire le suddette animazioni tipo
        // una enum class
        PlayerStats stats;
        PlayerMalus malus;

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

    private:
        void move(float dt, const bool* keys);
};


class Enemy : public Entity
{
    public:
        Enemy();

        EnemyStats stats;
        EnemyMalus malus;

        void Update(const Player& p, float dt);
    private:
        void move(const Player& p, float dt);
};
// da aggiungere tutti i tipi di nemici sopra è la classe base
