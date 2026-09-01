#pragma once

// #include "global.hpp"
#include "global.hpp"
#include <SDL3/SDL.h>
#include <string>
#include <mutex>

class Editor
{
    public:
        Editor();
        ~Editor();

        void Update(SDL_Event &e);
        void DrawWorld();
        void DrawGui();

        void run();

        void OpenImageDialog();
        vec2D SnapToNearbyPoint(vec2D worldPos, float radius);

    private:
        bool isRunning;
        Camera camera;

        // gestione asincrona del file scelto
        std::mutex fileMutex;
        std::string pendingImagePath;
        bool hasNewImage = false;

        static void SDLCALL FileDialogCallback(void* userdata, const char* const* filelist, int filter);
        void LoadImage(const std::string& path);

        SDL_Texture* currentTexture = nullptr;

        SDL_Cursor* cursorDefault;
        SDL_Cursor* cursorMove;
        DrawMode currentMode;
};
