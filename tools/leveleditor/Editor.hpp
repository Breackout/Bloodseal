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
        // --- BOTTONI E TOOLBARS ---
        GUI::Button back{10.0f, "back", colore};

        // Bottoni per lo stato "base"
        GUI::Button forme{10.0f, "forme", colore};
        GUI::Button nemici{10.0f, "nemici", colore};
        GUI::Button stanze{10.0f, "stanze", colore};
        GUI::Button altro{10.0f, "altro", colore};

        // Bottoni specifici per le sotto-sezioni
        GUI::Button rettangolo{10.0f, "rettangolo", colore};
        GUI::Button triangolo{10.0f, "triangolo", colore};

        GUI::Button apri{10.0f, "apri", colore};
        GUI::Button salva{10.0f, "salva", colore};

        // Una Toolbar per ogni statusBar
        GUI::Toolbar baseToolbar;
        GUI::Toolbar formeToolbar;
        GUI::Toolbar nemiciToolbar;
        GUI::Toolbar stanzeToolbar;
        GUI::Toolbar altroToolbar;

        // Funzione helper per ottenere la toolbar corrente in base a currentStatus
        GUI::Toolbar* GetCurrentToolbar();
};
