#pragma once

#include "Global.hpp"
#include <vector>
#include <string>
#include <cmath>

// --- operatori matematici minimi per vec2D (Global.hpp non li definisce) ---
inline vec2D operator+(const vec2D& a, const vec2D& b) { return { a.x + b.x, a.y + b.y }; }
inline vec2D operator-(const vec2D& a, const vec2D& b) { return { a.x - b.x, a.y - b.y }; }
inline vec2D operator*(const vec2D& a, float s)         { return { a.x * s, a.y * s }; }
inline float Dot(const vec2D& a, const vec2D& b)        { return a.x * b.x + a.y * b.y; }
inline float Length(const vec2D& a)                     { return std::sqrt(Dot(a, a)); }
inline vec2D Normalize(const vec2D& a)
{
    float l = Length(a);
    return l > 0.0001f ? vec2D{ a.x / l, a.y / l } : vec2D{ 0.0f, 0.0f };
}
inline vec2D Perp(const vec2D& a) { return { -a.y, a.x }; }

struct Triangle
{
    vec2D p[3];
};

class Polygon
{
    public:
        std::vector<vec2D> points;       // vertici originali (dal tuo tool)
        std::vector<Triangle> triangles; // scomposizione convessa, generata da Build()

        // Carica i punti da una stringa tipo quella che hai incollato ("x y\nx y\n...")
        static Polygon LoadFromString(const std::string& data);

        // Legge un file di testo (path relativo alla working directory dell'eseguibile)
        // e lo passa a LoadFromString.
        static Polygon LoadFromFile(const std::string& path);

        // Triangola il poligono (ear clipping). Va chiamata dopo aver riempito 'points'.
        void Build();
        void DebugDraw(SDL_Renderer* renderer, const vec2D& cameraOffset) const;
};

struct CollisionResult
{
    bool collided = false;
    vec2D mtv{ 0.0f, 0.0f }; // vettore da SOMMARE alla posizione del box per uscire dal poligono
};

// Testa un SDL_FRect (in world space, non camera space!) contro il poligono e
// ritorna il vettore di correzione minimo se c'è collisione.
CollisionResult ResolveAABBPolygon(const SDL_FRect& box, const Polygon& poly);
