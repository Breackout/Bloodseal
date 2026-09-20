#include "Game.hpp"
#include "Entity.hpp"
#include "Global.hpp"
#include "Collision.hpp"

Game::Game() :
    isRunning(true)
{
    // carica il primo livello (index 0) e setta subito i bounds della camera
    lvlManager.Start(0, player, camera);
}

void Game::Update(float dt, const bool* keys)
{
    if (!lvlManager.IsFading())
        player.Update(dt, keys);

    level* lvl = lvlManager.GetCurrentLevel();
    if (lvl)
    {
        CheckCollisionWithLevel(player, *lvl);
    }

    // controlla se il player ha raggiunto un bordo e cambia livello di conseguenza
    // (aggiorna anche i bounds della camera quando cambia livello)
    lvlManager.Update(player, camera, dt);

    camera.Update(player.pos, player.width, player.height, dt);
}

void Game::Draw()
{
    SDL_SetRenderScale(rend.GetRenderer(), camera.zoom, camera.zoom);

    lvlManager.Draw(camera);

    player.Draw(camera);

    SDL_SetRenderScale(rend.GetRenderer(), 1.0f, 1.0f);

    // overlay nero per il fade in/out tra livelli, disegnato SOPRA tutto e
    // fuori dallo zoom della camera cosi' copre sempre l'intero schermo
    Uint8 fadeAlpha = lvlManager.GetFadeAlpha();
    if (fadeAlpha > 0)
    {
        SDL_SetRenderDrawBlendMode(rend.GetRenderer(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(rend.GetRenderer(), 0, 0, 0, fadeAlpha);
        SDL_FRect fullScreen{ 0.0f, 0.0f, float(ScreenWidth), float(ScreenHeight) };
        SDL_RenderFillRect(rend.GetRenderer(), &fullScreen);
    }
}

void Game::run()
{
    SDL_Event e;
    Uint64 lastTime = SDL_GetTicks();
    keys = SDL_GetKeyboardState(nullptr);

    while (isRunning)
    {
        Uint64 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
                isRunning = false;

            if (e.type == SDL_EVENT_KEY_DOWN)
            {
                if (e.key.key == SDLK_ESCAPE)
                    isRunning = false;
            }
        }

            Update(dt, keys);


        SDL_SetRenderDrawColor(rend.GetRenderer(), 0, 0, 0, 255);
        SDL_RenderClear(rend.GetRenderer());

            Draw();

        SDL_RenderPresent(rend.GetRenderer());
    }
}
