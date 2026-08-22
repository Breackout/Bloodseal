#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Render
{
    public:
        Render(const char* title, int screenWidth, int ScreenHeight);
        ~Render();

        SDL_Window* GetWindow();
        SDL_Renderer* GetRenderer();

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
};
