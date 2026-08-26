#pragma once

#include <SDL3/SDL.h>

inline float GRAVITY = 1000.0f;
inline int ScreenWidth = 800;
inline int ScreenHeight = 600;
inline const char* title = "BloodSeal V0.1";

struct vec2D
{
    float x = 0.0f, y = 0.0f;
};


struct Rectangle { vec2D p[4]; bool isGround = false; };
struct Triangle { vec2D p[3]; bool isGround = false; };
