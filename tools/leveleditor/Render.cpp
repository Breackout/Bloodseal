#include "Render.hpp"
#include <cmath>
#include <cstdlib>

Render rend("level editor", 800, 600);

Render::Render(const char* title, int w, int h)
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("could not init SDL, ERROR: %s", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(title, w, h, SDL_WINDOW_RESIZABLE);
    if(window == nullptr)
    {
        SDL_Log("could create the window, ERROR: %s", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if(renderer == nullptr)
    {
        SDL_Log("could not create the renderer, ERROR: %s", SDL_GetError());
        exit(1);
    }

    if(!SDL_SetRenderVSync(renderer, 1))
        SDL_Log("could not init VSync, ERROR: %s", SDL_GetError());
}
Render::~Render()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SDL_Renderer* Render::GetRenderer()
{
    return renderer;
}

SDL_Window* Render::GetWindow()
{
    return window;
}

void Render::DrawThickLine(Camera& camera,vec2D a, vec2D b, SDL_Color color, float screenThickness = 2.0f)
{
    vec2D sa = WorldToScreen(a, camera);
    vec2D sb = WorldToScreen(b, camera);

    float dx = sb.x - sa.x;
    float dy = sb.y - sa.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) return;

    // normale perpendicolare alla linea, normalizzata
    float nx = -dy / len;
    float ny = dx / len;

    // spessore costante a schermo indipendentemente dallo zoom
    float half = (screenThickness / camera.zoom) * 0.5f;

    SDL_FColor c { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    SDL_Vertex verts[4] = {
        { { sa.x + nx * half, sa.y + ny * half }, c, {0, 0} },
        { { sa.x - nx * half, sa.y - ny * half }, c, {0, 0} },
        { { sb.x - nx * half, sb.y - ny * half }, c, {0, 0} },
        { { sb.x + nx * half, sb.y + ny * half }, c, {0, 0} },
    };

    int indices[6] = { 0, 1, 2, 0, 2, 3 };

    SDL_RenderGeometry(renderer, nullptr, verts, 4, indices, 6);
}
void Render::DrawPoint(Camera& camera, vec2D pos, float baseRadius = 4.0f)
{
    vec2D s = WorldToScreen(pos, camera);

    // Dividiamo per zoom così la dimensione a schermo rimane costante
    float currentRadius = baseRadius / camera.zoom;

    SDL_FRect point {
        s.x - currentRadius,
        s.y - currentRadius,
        currentRadius * 2.0f,
        currentRadius * 2.0f
    };
    SDL_RenderFillRect(renderer, &point);
}
void Render::DrawPlatform(Camera& camera, const platform& plat, bool closed)
{
    if(plat.points.empty()) return;

    SDL_Color lineColor { 0, 255, 0, 255 };
    for (size_t i = 0; i + 1 < plat.points.size(); i++)
    {
        DrawThickLine(camera, plat.points[i], plat.points[i + 1], lineColor);
    }

    if (closed && plat.points.size() > 2)
    {
        DrawThickLine(camera, plat.points.back(), plat.points.front(), lineColor);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(const vec2D &p : plat.points)
    {
        DrawPoint(camera, p);
    }
}
