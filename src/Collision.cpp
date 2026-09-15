#include "Collision.hpp"
#include "Entity.hpp"

#include <cmath>
#include <algorithm>
#include <limits>
#include <array>


// ---- Helper interni ----

// Converte una SDL_FRect nei suoi 4 punti (in ordine, senso orario)
static std::array<vec2D, 4> BoxToPoints(const SDL_FRect& box)
{
    return
    {
        vec2D{ box.x,          box.y },
        vec2D{ box.x + box.w,  box.y },
        vec2D{ box.x + box.w,  box.y + box.h },
        vec2D{ box.x,          box.y + box.h }
    };
}

// Proietta tutti i punti di un poligono su un asse e restituisce min/max
static void ProjectPolygon(const vec2D* poly, int count, const vec2D& axis, float& outMin, float& outMax)
{
    outMin = std::numeric_limits<float>::max();
    outMax = std::numeric_limits<float>::lowest();

    for (int i = 0; i < count; ++i)
    {
        float proj = poly[i].x * axis.x + poly[i].y * axis.y;
        outMin = std::min(outMin, proj);
        outMax = std::max(outMax, proj);
    }
}

// Centro (media dei vertici) di un poligono, usato per orientare l'MTV
static vec2D PolygonCenter(const vec2D* poly, int count)
{
    vec2D center{ 0.0f, 0.0f };
    for (int i = 0; i < count; ++i)
    {
        center.x += poly[i].x;
        center.y += poly[i].y;
    }
    center.x /= float(count);
    center.y /= float(count);
    return center;
}


// ---- SAT generico ----

CollisionResult CheckCollisionSAT(const vec2D* polyA, int countA, const vec2D* polyB, int countB)
{
    CollisionResult result;

    float minOverlap = std::numeric_limits<float>::max();
    vec2D smallestAxis{ 0.0f, 0.0f };

    // Testa tutti gli assi (normali ai lati) di un poligono.
    // Ritorna false appena trova un asse di separazione (= niente collisione).
    auto testAxesOf = [&](const vec2D* poly, int count) -> bool
    {
        for (int i = 0; i < count; ++i)
        {
            const vec2D& p1 = poly[i];
            const vec2D& p2 = poly[(i + 1) % count];

            vec2D edge{ p2.x - p1.x, p2.y - p1.y };
            vec2D axis{ -edge.y, edge.x }; // normale al lato

            float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
            if (len < 1e-6f)
                continue; // lato degenere, ignora

            axis.x /= len;
            axis.y /= len;

            float minA, maxA, minB, maxB;
            ProjectPolygon(polyA, countA, axis, minA, maxA);
            ProjectPolygon(polyB, countB, axis, minB, maxB);

            if (maxA < minB || maxB < minA)
                return false; // asse di separazione trovato -> nessuna collisione

            float overlap = std::min(maxA, maxB) - std::max(minA, minB);
            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                smallestAxis = axis;
            }
        }
        return true;
    };

    if (!testAxesOf(polyA, countA)) return result; // colliding resta false
    if (!testAxesOf(polyB, countB)) return result;

    // Nessun asse di separazione trovato -> i poligoni collidono.
    // Orienta l'MTV in modo che "spinga via" polyA da polyB.
    vec2D centerA = PolygonCenter(polyA, countA);
    vec2D centerB = PolygonCenter(polyB, countB);
    vec2D direction{ centerA.x - centerB.x, centerA.y - centerB.y };

    if (direction.x * smallestAxis.x + direction.y * smallestAxis.y < 0.0f)
    {
        smallestAxis.x = -smallestAxis.x;
        smallestAxis.y = -smallestAxis.y;
    }

    result.colliding = true;
    result.mtv = { smallestAxis.x * minOverlap, smallestAxis.y * minOverlap };
    return result;
}


// ---- Wrapper specifici ----

CollisionResult CheckCollisionAABBRect(const SDL_FRect& box, const Rectangle& rect)
{
    auto boxPoints = BoxToPoints(box);
    return CheckCollisionSAT(boxPoints.data(), 4, rect.p, 4);
}

CollisionResult CheckCollisionAABBTriangle(const SDL_FRect& box, const Triangle& tri)
{
    auto boxPoints = BoxToPoints(box);
    return CheckCollisionSAT(boxPoints.data(), 4, tri.p, 3);
}

void ResolvePlayerCollision(const CollisionResult& result, bool isGroundSurface, Entity& e)
{
    if (!result.colliding)
        return;

    e.pos.x += result.mtv.x;
    e.pos.y += result.mtv.y;

    float len = std::sqrt(result.mtv.x * result.mtv.x + result.mtv.y * result.mtv.y);
    if (len < 1e-6f)
        return;

    vec2D normal{ result.mtv.x / len, result.mtv.y / len };

    // Soglia di pendenza: normal.y molto negativo = superficie quasi orizzontale (pavimento/rampa dolce)
    // normal.y vicino a 0 = superficie verticale (muro)
    const float groundThreshold = 0.5f; // regola in base a quanto ripide vuoi le rampe percorribili

    if (normal.y < -groundThreshold)
    {
        // Pavimento o rampa percorribile: NON tocchiamo vel.x,
        // lo controlla completamente l'input del player in move()
        if (e.vel.y < 0.0f || true) // atterraggio: azzeriamo solo la componente verticale
            e.vel.y = 0.0f;

        if (isGroundSurface)
            e.isGround = true;
    }
    else if (normal.y > groundThreshold)
    {
        // Soffitto
        if (e.vel.y < 0.0f)
            e.vel.y = 0.0f;
    }
    else
    {
        // Muro laterale (normale prevalentemente orizzontale)
        e.vel.x = 0.0f;
    }
}

void CheckCollisionWithLevel(Entity& e, level& lvl)
{
    // Reimpostato ogni frame: verrà settato a true solo se troviamo
    // un contatto valido con una superficie isGround
    e.isGround = false;

    for (auto& rect : lvl.rects)
    {
        SDL_FRect box = e.GetWorldBox(); // ricalcolata ad ogni test, dopo eventuali risoluzioni precedenti
        CollisionResult res = CheckCollisionAABBRect(box, rect);
        ResolvePlayerCollision(res, rect.isGround, e);
    }

    for (auto& tri : lvl.tris)
    {
        SDL_FRect box = e.GetWorldBox();
        CollisionResult res = CheckCollisionAABBTriangle(box, tri);
        ResolvePlayerCollision(res, tri.isGround, e);
    }
}
