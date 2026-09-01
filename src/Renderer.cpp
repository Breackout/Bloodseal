#include "Render.hpp"
#include "SDL3_image/SDL_image.h"
#include "Global.hpp"

#include <cstdlib>

Render rend(title, ScreenWidth, ScreenHeight);

Render::Render(const char* title, int ScreenWidth, int ScreenHeight)
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("could not init SDL, ERROR: %s", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(title, ScreenWidth, ScreenHeight, SDL_WINDOW_ALWAYS_ON_TOP);
    if(window == nullptr)
    {
        SDL_Log("could not create the window, ERROR: %s", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if(renderer == nullptr)
    {
        SDL_Log("could not create the renderer, ERROR: %s", SDL_GetError());
        exit(1);
    }

    if(!SDL_SetRenderVSync(renderer, 1))
        SDL_Log("could not set VSync, ERROR: %s", SDL_GetError());
}

Render::~Render()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Render::drawThickLine(float x1, float y1, float x2, float y2, float thickness)
{
    float half = thickness / 2.0f;
    for (float offset = -half; offset <= half; offset += 1.0f)
    {
        SDL_RenderLine(renderer, x1, y1 + offset, x2, y2 + offset);
        SDL_RenderLine(renderer, x1 + offset, y1, x2 + offset, y2);
    }
}


SDL_Window* Render::GetWindow()
{
    return window;
}

SDL_Renderer* Render::GetRenderer()
{
    return renderer;
}



Texture::Texture() :
    texture( nullptr )
{}

Texture::~Texture()
{
    Destroy();
}

bool Texture::LoadFromFile(SDL_Renderer* renderer, const char* path)
{
    Destroy();

    texture = IMG_LoadTexture(renderer, path);
    if(texture == nullptr)
    {
        SDL_Log("could not load the image, Path: %s", path);
        return false;
    }

    return true;
}

void Texture::Destroy()
{
    SDL_DestroyTexture(texture);
    texture = nullptr;
}
