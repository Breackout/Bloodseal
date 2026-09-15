#include "Game.hpp"
#include "Entity.hpp"
#include "Global.hpp"
#include "Collision.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


// ==================== level ====================

level::level(const char* texture, const char* collision)
{
    map.LoadFromFile(rend.GetRenderer(), texture);
    LoadLevelCollisionInfo(collision);
    mapRect = {
        0.0f, // x
        0.0f, // y
        float(map.texture->w),
        float(map.texture->h),
    };
}
level::~level()
{
    map.Destroy();
}
void level::LoadLevelCollisionInfo(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        SDL_Log("Impossibile aprire il file di collisione: %s", path);
        return;
    }

    json data;
    try
    {
        file >> data;
    }
    catch (const json::parse_error& e)
    {
        SDL_Log("Errore parsing JSON (%s): %s", path, e.what());
        return;
    }

    for (const auto& platform : data)
    {
        bool isGround = platform.value("isGround", false);
        bool isRect   = platform.value("isRect", false);
        const auto& points = platform["points"];

        if (isRect)
        {
            if (points.size() != 4)
            {
                SDL_Log("Rettangolo con %zu punti invece di 4, saltato", points.size());
                continue;
            }

            Rectangle rect;
            rect.isGround = isGround;
            for (size_t i = 0; i < 4; ++i)
            {
                rect.p[i].x = points[i]["x"].get<float>();
                rect.p[i].y = points[i]["y"].get<float>();
            }
            rects.push_back(rect);
        }
        else
        {
            if (points.size() != 3)
            {
                SDL_Log("Triangolo con %zu punti invece di 3, saltato", points.size());
                continue;
            }

            Triangle tri;
            tri.isGround = isGround;
            for (size_t i = 0; i < 3; ++i)
            {
                tri.p[i].x = points[i]["x"].get<float>();
                tri.p[i].y = points[i]["y"].get<float>();
            }
            tris.push_back(tri);
        }
    }
}








// ==================== Game ====================

Game::Game() :
    isRunning(true),
    lvl("assets/map.png", "src/collisionData/map.json")
{
    camera.SetBounds(float(lvl.map.texture->w), float(lvl.map.texture->h));
}

void Game::DrawDebugCollisions()
{
    SDL_Renderer* renderer = rend.GetRenderer();

    // Disegna i lati di un poligono (array di vec2D) come linee, convertendo
    // dalle coordinate world a quelle schermo sottraendo la posizione della camera
    auto drawPolygonOutline = [&](const vec2D* points, int count, Uint8 r, Uint8 g, Uint8 b)
    {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);

        for (int i = 0; i < count; ++i)
        {
            const vec2D& p1 = points[i];
            const vec2D& p2 = points[(i + 1) % count];

            rend.drawThickLine(
                p1.x - camera.pos.x, p1.y - camera.pos.y,
                p2.x - camera.pos.x, p2.y - camera.pos.y,
                5.0f
            );
        }
    };

    for (const auto& rect : lvl.rects)
    {
        if (rect.isGround)
            drawPolygonOutline(rect.p, 4, 0, 255, 0);   // verde = ground
        else
            drawPolygonOutline(rect.p, 4, 255, 0, 0);   // rosso = non ground
    }

    for (const auto& tri : lvl.tris)
    {
        if (tri.isGround)
            drawPolygonOutline(tri.p, 3, 0, 255, 0);
        else
            drawPolygonOutline(tri.p, 3, 255, 0, 0);
    }

}

void Game::Update(float dt, const bool* keys)
{
    player.Update(dt, keys);
    e.Update(player, dt);
    CheckCollisionWithLevel(player, lvl);
    CheckCollisionWithLevel(e, lvl);
    camera.Update(player.pos, player.width, player.height, dt);
}

void Game::Draw()
{
    SDL_SetRenderScale(rend.GetRenderer(), camera.zoom, camera.zoom);

    lvl.mapRect.x = -camera.pos.x;
    lvl.mapRect.y = -camera.pos.y;
    SDL_RenderTexture(rend.GetRenderer(), lvl.map.texture, nullptr, &lvl.mapRect);

    player.Draw(camera);
    e.Draw(camera);
    DrawDebugCollisions();

    SDL_SetRenderScale(rend.GetRenderer(), 1.0f, 1.0f);
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
