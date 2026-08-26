#include "Game.hpp"
#include "Global.hpp"
#include "Collision.hpp"

#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Render rend(title, ScreenWidth, ScreenHeight);

// ==================== Camera ====================

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


// ==================== Player ====================

void Player::move(float dt, const bool* keys)
{
    moving = false;

    if (keys[SDL_SCANCODE_A])
    {
        vel.x -= acceleration * dt;
        moving = true;
    }
    if (keys[SDL_SCANCODE_D])
    {
        vel.x += acceleration * dt;
        moving = true;
    }

    // se non premi nulla si ferma (SOLO orizzontalmente: non tocchiamo vel.y,
    // altrimenti la gravità/il salto si "resetterebbero" ogni volta che lasci A/D)
    if (!moving)
        vel.x = 0.0f;

    // Clamp alla velocità massima orizzontale
    if (vel.x > maxSpeed)  vel.x = maxSpeed;
    if (vel.x < -maxSpeed) vel.x = -maxSpeed;

    if (keys[SDL_SCANCODE_SPACE] && isGround)
    {
        vel.y = jumpForce;
        isGround = false;
    }

    // La gravità si accumula sulla velocità verticale, non sostituisce il movimento
    vel.y += GRAVITY * dt;

    // Aggiorna la posizione in base alla velocità
    pos.x += vel.x;
    pos.y += vel.y * dt;
}


void Player::Update(float dt, const bool* keys)
{
    move(dt, keys);
}

void Player::Draw(const Camera& camera)
{
    // playerBox qui è puramente per il disegno: coordinate relative alla camera
    playerBox.x = pos.x - camera.pos.x;
    playerBox.y = pos.y - camera.pos.y;
    playerBox.w = width;
    playerBox.h = height;

    SDL_SetRenderDrawColor(rend.GetRenderer(), 0, 0, 0, 255);
    SDL_RenderFillRect(rend.GetRenderer(), &playerBox);
}


// ==================== level ====================

level::level(const char* path)
{
    map.LoadFromFile(rend.GetRenderer(), path);
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
    lvl("assets/map.png")
{
    lvl.LoadLevelCollisionInfo("src/collisionData/map1.json");
    camera.SetBounds(float(lvl.map.texture->w), float(lvl.map.texture->h));
}

void Game::ResolvePlayerCollision(const CollisionResult& result, bool isGroundSurface)
{
    if (!result.colliding)
        return;

    player.pos.x += result.mtv.x;
    player.pos.y += result.mtv.y;

    float len = std::sqrt(result.mtv.x * result.mtv.x + result.mtv.y * result.mtv.y);
    if (len < 1e-6f)
        return;

    vec2D normal{ result.mtv.x / len, result.mtv.y / len };

    // Soglia di pendenza: normal.y molto negativo = superficie quasi orizzontale (pavimento/rampa dolce)
    // normal.y vicino a 0 = superficie verticale (muro)
    const float groundThreshold = 0.5f; // regola in base a quanto ripide vuoi le rampe percorribili

    if (normal.y < -groundThreshold)
    {
        // Pavimento o rampa percorribile: NON tocchiamo vel.x,
        // lo controlla completamente l'input del player in move()
        if (player.vel.y < 0.0f || true) // atterraggio: azzeriamo solo la componente verticale
            player.vel.y = 0.0f;

        if (isGroundSurface)
            player.isGround = true;
    }
    else if (normal.y > groundThreshold)
    {
        // Soffitto
        if (player.vel.y < 0.0f)
            player.vel.y = 0.0f;
    }
    else
    {
        // Muro laterale (normale prevalentemente orizzontale)
        player.vel.x = 0.0f;
    }
}

void Game::CheckCollisionWithLevel()
{
    // Reimpostato ogni frame: verrà settato a true solo se troviamo
    // un contatto valido con una superficie isGround
    player.isGround = false;

    for (auto& rect : lvl.rects)
    {
        SDL_FRect box = player.GetWorldBox(); // ricalcolata ad ogni test, dopo eventuali risoluzioni precedenti
        CollisionResult res = CheckCollisionAABBRect(box, rect);
        ResolvePlayerCollision(res, rect.isGround);
    }

    for (auto& tri : lvl.tris)
    {
        SDL_FRect box = player.GetWorldBox();
        CollisionResult res = CheckCollisionAABBTriangle(box, tri);
        ResolvePlayerCollision(res, tri.isGround);
    }
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

            SDL_RenderLine(renderer,
                p1.x - camera.pos.x, p1.y - camera.pos.y,
                p2.x - camera.pos.x, p2.y - camera.pos.y);
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
    CheckCollisionWithLevel();
    camera.Update(player.pos, player.width, player.height, dt);
}

void Game::Draw()
{
    SDL_SetRenderScale(rend.GetRenderer(), camera.zoom, camera.zoom);

    lvl.mapRect.x = -camera.pos.x;
    lvl.mapRect.y = -camera.pos.y;
    SDL_RenderTexture(rend.GetRenderer(), lvl.map.texture, nullptr, &lvl.mapRect);

    player.Draw(camera);
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
