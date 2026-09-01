#pragma once

#include "Game.hpp"
#include "Global.hpp"


struct CollisionResult
{
    bool colliding = false;
    vec2D mtv; // Minimum Translation Vector: quanto e in che direzione spostare
               // il primo poligono (box) per risolvere la compenetrazione
};


// Test SAT generico tra due poligoni convessi (usato internamente dalle funzioni sotto,
// ma esposto nel caso servisse anche per altri tipi di forme in futuro)
CollisionResult CheckCollisionSAT(const vec2D* polyA, int countA, const vec2D* polyB, int countB);

// Collisione AABB (box) vs rettangolo
CollisionResult CheckCollisionAABBRect(const SDL_FRect& box, const Rectangle& rect);

// Collisione AABB (box) vs triangolo
CollisionResult CheckCollisionAABBTriangle(const SDL_FRect& box, const Triangle& tri);

// Controlla il player contro tutti i rects/tris di tutti i livelli caricati
void CheckCollisionWithLevel(Player& player, level& lvl);

// Applica l'MTV di una singola collisione al player e aggiorna vel/isGround di conseguenza
void ResolvePlayerCollision(const CollisionResult& result, bool isGroundSurface, Player& player);
