#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Render
{
    public:
        Render(const char* title, int screenWidth, int ScreenHeight);
        ~Render();

        void drawThickLine(float x1, float y1, float x2, float y2, float thickness);

        SDL_Window* GetWindow();
        SDL_Renderer* GetRenderer();

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
};

struct Texture
{
    Texture();
    ~Texture();

    bool LoadFromFile(SDL_Renderer* renderer, const char* path);
    void Destroy();

    SDL_Texture* texture;
};

extern Render rend;
