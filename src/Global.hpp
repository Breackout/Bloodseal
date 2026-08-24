#pragma once

#include <SDL3/SDL.h>

inline float GRAVITY = 500.0f;
inline int ScreenWidth = 800;
inline int ScreenHeight = 600;
inline const char* title = "Game";

struct vec2D
{
    float x = 0.0f, y = 0.0f;
};

struct level
{
    level() = default;
    level(float w, float h) :
        levelW(w), levelH(h)
    {}

    float levelW = 0.0f;
    float levelH = 0.0f;
};
