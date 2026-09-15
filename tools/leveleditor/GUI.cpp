#include "GUI.hpp"
#include "Render.hpp"
#include "global.hpp"
#include <functional>

GUI::Button::Button(const float margin, std::string text, SDL_Color c) :
    m_margin(margin),
    m_buttonColor(c),
    m_text(text)
{
    m_buttonRect = { 0.0f, 0.0f, 0.0f, 0.0f };
}

void GUI::Button::Draw()
{
    SDL_Color c = m_buttonColor;
    if(isHover)
    {
        c.r -= 50;
        c.g -= 50;
        c.b -= 50;
    }

    SDL_SetRenderDrawColor(rend.GetRenderer(), c.r, c.g, c.b, 255);
    SDL_RenderFillRect(rend.GetRenderer(), &m_buttonRect);

    // Testo centrato (approssimativo, font 8x8 px)
    SDL_SetRenderDrawColor(rend.GetRenderer(), 255, 255, 255, 255);
    float testoX = m_buttonRect.x + m_buttonRect.w / 2 - float(m_text.size() * 8) / 2;
    float testoY = m_buttonRect.y + m_buttonRect.h / 2 - 4;
    SDL_RenderDebugText(rend.GetRenderer(), testoX, testoY, m_text.c_str());
}

void GUI::Button::Update(bool& isMouseButtonDown, vec2D mp, const SDL_FRect& anchor, std::function<void()> function)
{
    // ricalcola posizione e size ogni frame cosi' il bottone resta ancorato
    // dentro "anchor" (es. la barra "base") con un margine fisso, anche se
    // la finestra (e quindi anchor) cambia dimensione
    float h = anchor.h - m_margin * 2.0f;
    if (h < 0.0f) h = 0.0f;
    float w = h; // bottone quadrato

    m_buttonRect = {
        anchor.x + m_margin,
        anchor.y + m_margin,
        w,
        h
    };

    isHover = (mp.x >= m_buttonRect.x && mp.x <= m_buttonRect.x + m_buttonRect.w
            && mp.y >= m_buttonRect.y && mp.y <= m_buttonRect.y + m_buttonRect.h);

    if (isMouseButtonDown && isHover && function)
    {
        function();
        isMouseButtonDown = false;
    }
}

void GUI::Toolbar::Add(Button& button, std::function<void()> onClick)
{
    m_entries.push_back({ &button, onClick });
}

void GUI::Toolbar::Update(bool& isMouseButtonDown, vec2D mp, const SDL_FRect& row)
{
    SDL_FRect anchor = row; // il primo bottone parte dal bordo sinistro di "row"

    for (Entry& entry : m_entries)
    {
        entry.button->Update(isMouseButtonDown, mp, anchor, entry.onClick);

        // il prossimo bottone si aggancia al bordo destro di questo:
        // il suo m_margin fara' automaticamente da spaziatura
        anchor.x = entry.button->m_buttonRect.x + entry.button->m_buttonRect.w;
        anchor.w = row.x + row.w - anchor.x;
    }
}

void GUI::Toolbar::Draw()
{
    for (Entry& entry : m_entries)
    {
        entry.button->Draw();
    }
}
