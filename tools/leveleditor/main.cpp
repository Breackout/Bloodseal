// LevelEditor.cpp
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <algorithm>

struct vec2D { float x = 0.0f, y = 0.0f; };
struct Platform { std::vector<vec2D> points; };

std::vector<Platform> platforms;
Platform currentPlatform;

vec2D cameraPos = { 0.0f, 0.0f };
float zoom = 1.0f;
const float zoomStep = 0.1f;
const float minZoom = 0.1f;
const float maxZoom = 5.0f;

bool spaceHeld = false;
bool isPanning = false;
vec2D panStartMouse;
vec2D panStartCamera;

// converte coordinate schermo -> coordinate mondo (tiene conto di camera e zoom)
vec2D ScreenToWorld(float sx, float sy)
{
    return { sx / zoom + cameraPos.x, sy / zoom + cameraPos.y };
}

// converte coordinate mondo -> coordinate schermo (per disegnare)
vec2D WorldToScreen(float wx, float wy)
{
    return { (wx - cameraPos.x) * zoom, (wy - cameraPos.y) * zoom };
}

void ZoomAt(float screenX, float screenY, float newZoom)
{
    newZoom = std::clamp(newZoom, minZoom, maxZoom);

    // il punto mondo sotto il cursore, prima dello zoom
    vec2D worldBefore = ScreenToWorld(screenX, screenY);

    zoom = newZoom;

    // il punto mondo sotto il cursore, dopo lo zoom (con la vecchia camera)
    vec2D worldAfter = ScreenToWorld(screenX, screenY);

    // sposta la camera per compensare, cosi il punto sotto il cursore resta fermo
    cameraPos.x += worldBefore.x - worldAfter.x;
    cameraPos.y += worldBefore.y - worldAfter.y;
}

void DrawPoint(SDL_Renderer* renderer, float wx, float wy, float radius = 4.0f)
{
    vec2D s = WorldToScreen(wx, wy);
    float r = radius * zoom;
    SDL_FRect rect = { s.x - r, s.y - r, r * 2.0f, r * 2.0f };
    SDL_RenderFillRect(renderer, &rect);
}

void DrawPlatform(SDL_Renderer* renderer, const Platform& plat, bool closed)
{
    if (plat.points.empty()) return;

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (size_t i = 0; i + 1 < plat.points.size(); i++)
    {
        vec2D a = WorldToScreen(plat.points[i].x, plat.points[i].y);
        vec2D b = WorldToScreen(plat.points[i + 1].x, plat.points[i + 1].y);
        SDL_RenderLine(renderer, a.x, a.y, b.x, b.y);
    }
    if (closed && plat.points.size() > 2)
    {
        vec2D a = WorldToScreen(plat.points.back().x, plat.points.back().y);
        vec2D b = WorldToScreen(plat.points.front().x, plat.points.front().y);
        SDL_RenderLine(renderer, a.x, a.y, b.x, b.y);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (auto& p : plat.points)
        DrawPoint(renderer, p.x, p.y);
}

void SavePlatforms(const char* filename)
{
    std::ofstream out(filename);
    if (!out.is_open())
    {
        SDL_Log("Impossibile aprire il file per salvare: %s", filename);
        return;
    }

    // i punti sono già salvati in coordinate MONDO (vedi ScreenToWorld sul click),
    // quindi qui non serve nessuna conversione
    out << platforms.size() << "\n";
    for (auto& plat : platforms)
    {
        out << plat.points.size() << "\n";
        for (auto& p : plat.points)
            out << p.x << " " << p.y << "\n";
    }

    SDL_Log("Salvato in %s (%zu piattaforme)", filename, platforms.size());
}

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init fallito: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Level Editor", 1280, 720, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    SDL_Texture* mapTexture = IMG_LoadTexture(renderer, "assets/map.png");
    if (!mapTexture)
        SDL_Log("Impossibile caricare l'immagine: %s", SDL_GetError());

    float mapW = 0, mapH = 0;
    if (mapTexture)
        SDL_GetTextureSize(mapTexture, &mapW, &mapH);

    // cursori creati una sola volta, non ogni frame
    SDL_Cursor* defaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    SDL_Cursor* moveCursor    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    bool cursorIsMove = false;

    bool running = true;
    SDL_Event e;
    float mouseX = 0.0f, mouseY = 0.0f;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
                running = false;

            if (e.type == SDL_EVENT_MOUSE_MOTION)
            {
                mouseX = e.motion.x;
                mouseY = e.motion.y;

                if (isPanning)
                {
                    // quanto si è mosso il mouse in pixel schermo -> convertilo in mondo
                    float dx = (mouseX - panStartMouse.x) / zoom;
                    float dy = (mouseY - panStartMouse.y) / zoom;
                    cameraPos.x = panStartCamera.x - dx;
                    cameraPos.y = panStartCamera.y - dy;
                }
            }

            if (e.type == SDL_EVENT_KEY_DOWN)
            {
                switch (e.key.key)
                {
                    case SDLK_SPACE:
                        spaceHeld = true;
                        break;

                    case SDLK_ESCAPE:
                        running = false;
                        break;

                    case SDLK_RETURN:
                        if (currentPlatform.points.size() > 1)
                        {
                            platforms.push_back(currentPlatform);
                            currentPlatform = Platform{};
                            SDL_Log("Piattaforma chiusa (%zu punti)", platforms.back().points.size());
                        }
                        break;

                    case SDLK_BACKSPACE:
                        if (!currentPlatform.points.empty())
                            currentPlatform.points.pop_back();
                        else if (!platforms.empty())
                        {
                            currentPlatform = platforms.back();
                            platforms.pop_back();
                            if (!currentPlatform.points.empty())
                                currentPlatform.points.pop_back();
                        }
                        break;

                    case SDLK_S:
                        SavePlatforms("tools/leveleditor/levelData.txt");
                        break;

                    // zoom in, centrato sul cursore
                    case SDLK_P:
                        ZoomAt(mouseX, mouseY, zoom + zoomStep);
                        break;

                    // zoom out, centrato sul cursore
                    case SDLK_L:
                        ZoomAt(mouseX, mouseY, zoom - zoomStep);
                        break;
                }
            }

            if (e.type == SDL_EVENT_KEY_UP)
            {
                if (e.key.key == SDLK_SPACE)
                {
                    spaceHeld = false;
                    isPanning = false;
                }
            }

            // click sinistro: pan se spazio è premuto, altrimenti piazza un punto
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT)
            {
                if (spaceHeld)
                {
                    isPanning = true;
                    panStartMouse = { e.button.x, e.button.y };
                    panStartCamera = cameraPos;
                }
                else
                {
                    vec2D worldPos = ScreenToWorld(e.button.x, e.button.y);
                    currentPlatform.points.push_back(worldPos);
                }
            }

            if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT)
            {
                isPanning = false;
            }

            // zoom anche con la rotellina, per comodità (opzionale)
            if (e.type == SDL_EVENT_MOUSE_WHEEL)
            {
                ZoomAt(mouseX, mouseY, zoom + e.wheel.y * zoomStep);
            }
        }

        // aggiorna il cursore solo quando cambia stato, non ogni frame
        bool shouldBeMove = spaceHeld;
        if (shouldBeMove != cursorIsMove)
        {
            SDL_SetCursor(shouldBeMove ? moveCursor : defaultCursor);
            cursorIsMove = shouldBeMove;
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        if (mapTexture)
        {
            vec2D topLeft = WorldToScreen(0.0f, 0.0f);
            SDL_FRect dst = { topLeft.x, topLeft.y, mapW * zoom, mapH * zoom };
            SDL_RenderTexture(renderer, mapTexture, nullptr, &dst);
        }

        for (auto& plat : platforms)
            DrawPlatform(renderer, plat, true);

        DrawPlatform(renderer, currentPlatform, false);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyCursor(moveCursor);
    SDL_DestroyCursor(defaultCursor);
    SDL_DestroyTexture(mapTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
