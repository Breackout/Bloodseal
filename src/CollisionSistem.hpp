#pragma once

#include "Global.hpp"

#include <SDL3/SDL.h>
#include <algorithm>

enum class CollisionSide
{
    None,
    Top,
    Bottom,
    Left,
    Right
};

inline bool CheckCollisionRect(const SDL_FRect& a, const SDL_FRect& b)
{
    return(
        a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y
    );
}

inline CollisionSide ResolveCollisionRect(SDL_FRect& player, const SDL_FRect& obstacle, vec2D& vel)
{
    if (!CheckCollisionRect(player, obstacle))
        return CollisionSide::None;

    // calcola l'overlap su entrambi gli assi
    float overlapLeft   = (player.x + player.w) - obstacle.x;
    float overlapRight  = (obstacle.x + obstacle.w) - player.x;
    float overlapTop    = (player.y + player.h) - obstacle.y;
    float overlapBottom = (obstacle.y + obstacle.h) - player.y;

    // il minimo overlap indica da che lato è avvenuta la collisione
    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    if (minOverlapX < minOverlapY)
    {
        // collisione orizzontale
        if (overlapLeft < overlapRight)
        {
            player.x -= overlapLeft;
            vel.x = 0.0f;
            return CollisionSide::Right; // collisioni destra
        }
        else
        {
            player.x += overlapRight;
            vel.x = 0.0f;
            return CollisionSide::Left; // collisioni sinistra
        }
    }
    else
    {
        // collisione verticale
        if (overlapTop < overlapBottom)
        {
            player.y -= overlapTop;
            vel.y = 0.0f;
            return CollisionSide::Bottom; // collisioni sotto ( sprite player )
        }
        else
        {
            player.y += overlapBottom;
            vel.y = 0.0f;
            return CollisionSide::Top;  // collisioni sopra ( sprite player )
        }
    }
}
