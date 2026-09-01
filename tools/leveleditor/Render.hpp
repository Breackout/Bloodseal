#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Editor.hpp"
#include "global.hpp"

class Render
{
    public:
        Render(const char* title, int w, int h);
        ~Render();

        SDL_Window* GetWindow();
        SDL_Renderer* GetRenderer();

        void DrawThickLine(Camera& camera,vec2D a, vec2D b, SDL_Color color, float screenThickness);
        void DrawPoint(Camera& camera, vec2D pos, float baseRadius);
        void DrawPlatform(Camera& camera, const platform& plat, bool closed);

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
};

extern Render rend;
