#pragma once

#include "Render.hpp"
#include "Global.hpp"
#include "Collision.hpp"
#include <vector>

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
        float width = 100.0f;
        float height = 100.0f;

        // usato SOLO per il rendering (coordinate schermo, relative alla camera)
        SDL_FRect playerBox = { pos.x, pos.y, width, height };

        // variabili per il movimento
        vec2D vel = { 0.0f, 0.0f };
        float acceleration = 25.0f;
        float maxSpeed = 5.0f;
        bool moving = false;
        float jumpForce = -600.0f;
        bool isGround = false;

        void Update(float dt, const bool* keys);
        void Draw(const Camera &camera);

        // Bounding box in world-space (NON relativa alla camera), usata per le collisioni
        SDL_FRect GetWorldBox() const
        {
            return SDL_FRect{ pos.x, pos.y, width, height };
        }

    private:
        void move(float dt, const bool* keys);
        void dash(float dt, const bool* keys);
};



struct level
{
    Texture map;
    SDL_FRect mapRect;

    std::vector<Rectangle> rects;
    std::vector<Triangle> tris;

    void LoadLevelCollisionInfo(const char* path);

    level(const char* path);
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
        const bool* keys;
        Camera camera;
        level lvl;


        // Controlla il player contro tutti i rects/tris di tutti i livelli caricati
        void CheckCollisionWithLevel();

        // Applica l'MTV di una singola collisione al player e aggiorna vel/isGround di conseguenza
        void ResolvePlayerCollision(const CollisionResult& result, bool isGroundSurface);

        void DrawDebugCollisions();
};
