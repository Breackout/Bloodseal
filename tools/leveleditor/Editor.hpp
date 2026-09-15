#pragma once

// #include "global.hpp"
#include "GUI.hpp"
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
        void UI();

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
        statusBar currentStatus;
        // ui stuff
        SDL_Color colore = {150, 150, 150, 255};
        SDL_FRect base;
        GUI::Button back{10.0f, "back", colore};
        GUI::Button next{10.0f, "next", colore}; // esempio di secondo bottone
        GUI::Toolbar toolbar; // dispone back/next in fila dentro "base", niente rettangoli manuali
};
