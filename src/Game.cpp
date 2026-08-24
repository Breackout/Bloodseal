#include "Game.hpp"
#include "Collision.hpp"
#include "Global.hpp"
#include "SDL3_image/SDL_image.h"
#include <cmath>

Render rend(title, ScreenWidth, ScreenHeight);

bool RectangleCollision(const SDL_FRect& r1, const SDL_FRect& r2) {
    return r1.x < r2.x + r2.w &&
           r1.x + r1.w > r2.x &&
           r1.y < r2.y + r2.h &&
           r1.y + r1.h > r2.y;
}

void Camera::SetBounds(float worldW, float worldH)
{
    worldWidth = worldW;
    worldHeight = worldH;
}

void Camera::Update(const vec2D& targetPos, float targetW, float targetH, float dt)
{
    float visibleWidth  = ScreenWidth  / zoom;
    float visibleHeight = ScreenHeight / zoom;

    float desiredX = (targetPos.x + targetW / 2.0f) - visibleWidth / 2.0f;
    float desiredY = (targetPos.y + targetH / 2.0f) - visibleHeight / 2.0f;

    float t = 1.0f - std::exp(-smoothSpeed * dt);
    pos.x += (desiredX - pos.x) * t;
    pos.y += (desiredY - pos.y) * t;

    if (worldWidth > 0.0f && worldHeight > 0.0f)
    {
        if (worldWidth <= visibleWidth)
            pos.x = (worldWidth - visibleWidth) * 0.5f; // mondo più piccolo della vista -> centra
        else
        {
            if (pos.x < 0.0f) pos.x = 0.0f;
            if (pos.x > worldWidth - visibleWidth) pos.x = worldWidth - visibleWidth;
        }

        if (worldHeight <= visibleHeight)
            pos.y = (worldHeight - visibleHeight) * 0.5f;
        else
        {
            if (pos.y < 0.0f) pos.y = 0.0f;
            if (pos.y > worldHeight - visibleHeight) pos.y = worldHeight - visibleHeight;
        }
    }
}



void Player::move(float dt, const bool* keys)
{
    moving = false;

    if(keys[SDL_SCANCODE_A])
    {
        vel.x -= acceleration * dt;
        moving = true;
    }
    if(keys[SDL_SCANCODE_D])
    {
        vel.x += acceleration * dt;
        moving = true;
    }

    // se non premi nulla si ferma
    if (!moving)
    {
        vel = { 0.0f, 0.0f };
    }

    // Clamp alla velocità massima (funziona sia per positivo che negativo)
    if (vel.x > maxSpeed)  vel.x = maxSpeed;
    if (vel.x < -maxSpeed) vel.x = -maxSpeed;

    // Aggiorna la posizione in base alla velocità
    pos.x += vel.x;
    pos.y += GRAVITY * dt;
}


void Player::Update(float dt, const bool* keys)
{
    move(dt, keys);
}
void Player::Draw(const Camera& camera)
{
    playerBox.x = pos.x - camera.pos.x;
    playerBox.y = pos.y - camera.pos.y;

    SDL_SetRenderDrawColor(rend.GetRenderer(), 0, 0, 0, 255);
    SDL_RenderFillRect(rend.GetRenderer(), &playerBox);
}




Game::Game() :
    isRunning(true),
    map(IMG_LoadTexture(rend.GetRenderer(), "assets/map.png"))
{
    if (map == nullptr)
    {
        SDL_Log("could not load the assets proprelly, ERROR: %s", SDL_GetError());
        return; // evita di dereferenziare map più sotto
    }

    mapRect = { 0.0f, 0.0f, float(map->w), float(map->h) };
    camera.SetBounds(mapRect.w, mapRect.h);

    ground = Polygon::LoadFromFile("tools/leveleditor/levelData.txt"); // <-- LoadFromFile, non LoadFromString
}

Game::~Game()
{
    SDL_DestroyTexture(map);
}

void Game::Update(float dt, const bool* keys)
{
    player.Update(dt, keys);

    SDL_FRect worldBox = { player.pos.x, player.pos.y, 100.0f, 100.0f };
    CollisionResult res = ResolveAABBPolygon(worldBox, ground);
    if (res.collided)
    {
        player.pos.x += res.mtv.x;
        player.pos.y += res.mtv.y;
        if (res.mtv.y < 0.0f) player.vel.y = 0.0f; // player è "a terra" quando la correzione spinge verso l'alto
    }

    camera.Update(player.pos, 100.0f, 100.0f, dt);
}

void Game::Draw()
{
    SDL_SetRenderScale(rend.GetRenderer(), camera.zoom, camera.zoom);

    SDL_FRect mapDrawRect = {
        mapRect.x - camera.pos.x,
        mapRect.y - camera.pos.y,
        mapRect.w,
        mapRect.h
    };

    SDL_RenderTexture(rend.GetRenderer(), map, nullptr, &mapDrawRect);
    player.Draw(camera);

    SDL_SetRenderScale(rend.GetRenderer(), 1.0f, 1.0f);
}

void Game::run()
{
    SDL_Event e;
    Uint64 lastTime = SDL_GetTicks();
    keys = SDL_GetKeyboardState(nullptr);

    while(isRunning)
    {
        Uint64 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&e))
        {
            if(e.type == SDL_EVENT_QUIT || keys[SDL_SCANCODE_ESCAPE])
                isRunning = false;
        }

            Update(dt, keys);


        SDL_SetRenderDrawColor(rend.GetRenderer(), 0, 0, 0, 255);
        SDL_RenderClear(rend.GetRenderer());

            Draw();

        SDL_RenderPresent(rend.GetRenderer());
    }

}
