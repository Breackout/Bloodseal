#pragma once

#include "global.hpp"
#include <SDL3/SDL.h>
#include <functional>
#include <string>
#include <vector>


namespace GUI
{
    struct Button
    {
        // margin = distanza in px che il bottone deve mantenere dai bordi
        // del rettangolo di ancoraggio (es. "base") sia in x che in y
        Button(const float margin, std::string text, SDL_Color);
        void Draw();
        void Update(bool& isMouseButtonDown, vec2D mp, const SDL_FRect& anchor, std::function<void()> function);

        float m_margin;
        SDL_FRect m_buttonRect;
        SDL_Color m_buttonColor;
        std::string m_text;
        bool isHover = false;
    };

    // Dispone in fila orizzontale una lista di bottoni dentro un unico
    // rettangolo (row, es. "base"). Ogni bottone viene agganciato al bordo
    // destro del precedente: il m_margin di ciascun Button fa automaticamente
    // da spaziatura, quindi non serve creare un SDL_FRect per ogni bottone.
    struct Toolbar
    {
        struct Entry
        {
            Button* button;
            std::function<void()> onClick;
        };

        void Add(Button& button, std::function<void()> onClick);
        void Update(bool& isMouseButtonDown, vec2D mp, const SDL_FRect& row);
        void Draw();

        std::vector<Entry> m_entries;
    };
}
