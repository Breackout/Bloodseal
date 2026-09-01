#pragma once

#include <vector>

struct vec2D
{
    float x = 0.0f;
    float y = 0.0f;
};

struct platform
{
    std::vector<vec2D> points;
    bool isGround = true;
    bool isRect = false;
};

struct Camera
{
    vec2D pos = { 0.0f, 0.0f };
    float zoom = 1.0f;

    void MoveCamera();
};

inline int ScreenWidth = 800;
inline int ScreenHeight = 600;

inline std::vector<platform> platforms;
inline platform currentPlatform;

inline vec2D mousePos = { 0.0f, 0.0f };
inline vec2D oldMousePos = { 0.0f, 0.0f };
inline bool isSpacePressed = false;
inline bool isLeftButtonPressed = false;

enum class DrawMode { Rectangle, Triangle };
inline bool isDrawingShape = false;
inline vec2D shapeOrigin = { 0.0f, 0.0f };

vec2D WorldToScreen(vec2D worldPos, Camera& camera);
vec2D ScreenToWorld(vec2D screenPos, Camera& camera);
void SavePlatform(const char* path);
