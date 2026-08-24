#include "Collision.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>

static float Cross(const vec2D& a, const vec2D& b) { return a.x * b.y - a.y * b.x; }

static bool PointInTriangle(const vec2D& p, const vec2D& a, const vec2D& b, const vec2D& c)
{
    float d1 = Cross(b - a, p - a);
    float d2 = Cross(c - b, p - b);
    float d3 = Cross(a - c, p - c);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(hasNeg && hasPos);
}

Polygon Polygon::LoadFromString(const std::string& data)
{
    Polygon poly;
    std::istringstream iss(data);
    float x, y;
    while (iss >> x >> y)
        poly.points.push_back({ x, y });

    poly.Build();
    return poly;
}

Polygon Polygon::LoadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        SDL_Log("Polygon::LoadFromFile - impossibile aprire il file: %s", path.c_str());
        return Polygon{};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return LoadFromString(buffer.str());
}

void Polygon::Build()
{
    triangles.clear();
    if (points.size() < 3) return;

    std::vector<vec2D> verts = points;

    // assicura che i vertici siano in ordine CCW (area con segno positiva)
    float area = 0.0f;
    for (size_t i = 0; i < verts.size(); ++i)
    {
        const vec2D& a = verts[i];
        const vec2D& b = verts[(i + 1) % verts.size()];
        area += a.x * b.y - b.x * a.y;
    }
    if (area < 0.0f) std::reverse(verts.begin(), verts.end());

    std::vector<int> idx(verts.size());
    for (size_t i = 0; i < idx.size(); ++i) idx[i] = (int)i;

    int guard = 0;
    while (idx.size() > 3 && guard < 100000)
    {
        guard++;
        bool earFound = false;

        for (size_t i = 0; i < idx.size(); ++i)
        {
            int iPrev = idx[(i + idx.size() - 1) % idx.size()];
            int iCurr = idx[i];
            int iNext = idx[(i + 1) % idx.size()];

            const vec2D& a = verts[iPrev];
            const vec2D& b = verts[iCurr];
            const vec2D& c = verts[iNext];

            if (Cross(b - a, c - b) <= 0.0f) continue; // vertice riflesso, non può essere un'orecchia

            bool anyInside = false;
            for (size_t j = 0; j < idx.size(); ++j)
            {
                int vIdx = idx[j];
                if (vIdx == iPrev || vIdx == iCurr || vIdx == iNext) continue;
                if (PointInTriangle(verts[vIdx], a, b, c)) { anyInside = true; break; }
            }
            if (anyInside) continue;

            triangles.push_back({ a, b, c });
            idx.erase(idx.begin() + i);
            earFound = true;
            break;
        }

        if (!earFound) break; // poligono non semplice / dati sporchi: evita loop infinito
    }

    if (idx.size() == 3)
        triangles.push_back({ verts[idx[0]], verts[idx[1]], verts[idx[2]] });
}


// SAT tra un SDL_FRect e un triangolo. Ritorna true + mtv se in collisione.
static bool TestBoxTriangle(const SDL_FRect& box, const Triangle& tri, vec2D& outMtv)
{
    vec2D boxCorners[4] = {
        { box.x,         box.y },
        { box.x + box.w, box.y },
        { box.x + box.w, box.y + box.h },
        { box.x,         box.y + box.h }
    };

    vec2D axes[5] = {
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        Normalize(Perp(tri.p[1] - tri.p[0])),
        Normalize(Perp(tri.p[2] - tri.p[1])),
        Normalize(Perp(tri.p[0] - tri.p[2]))
    };

    float minOverlap = 1e9f;
    vec2D mtvAxis{ 0.0f, 0.0f };

    for (const vec2D& axis : axes)
    {
        if (Length(axis) < 0.0001f) continue;

        float boxMin = 1e9f, boxMax = -1e9f;
        for (const auto& c : boxCorners)
        {
            float p = Dot(c, axis);
            boxMin = std::min(boxMin, p);
            boxMax = std::max(boxMax, p);
        }

        float triMin = 1e9f, triMax = -1e9f;
        for (const auto& p3 : tri.p)
        {
            float p = Dot(p3, axis);
            triMin = std::min(triMin, p);
            triMax = std::max(triMax, p);
        }

        float overlap = std::min(boxMax, triMax) - std::max(boxMin, triMin);
        if (overlap <= 0.0f) return false; // trovato asse di separazione -> nessuna collisione

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            mtvAxis = axis;

            // Calcola i centri nello spazio 2D (vettoriale)
            vec2D centerBox = { box.x + box.w * 0.5f, box.y + box.h * 0.5f };
            vec2D centerTri = { (tri.p[0].x + tri.p[1].x + tri.p[2].x) / 3.0f,
                                (tri.p[0].y + tri.p[1].y + tri.p[2].y) / 3.0f };

            // Direzione dal centro del triangolo al centro del player
            vec2D dir = centerBox - centerTri;
            if (Dot(mtvAxis, dir) < 0.0f)
            {
                mtvAxis = mtvAxis * -1.0f;
            }
        }
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            mtvAxis = axis;

            // orienta l'asse in modo che la correzione spinga il box FUORI dal triangolo
            float boxCenter = (boxMin + boxMax) * 0.5f;
            float triCenter = (triMin + triMax) * 0.5f;
            if (boxCenter < triCenter) mtvAxis = mtvAxis * -1.0f;
        }
    }

    outMtv = mtvAxis * minOverlap;
    return true;
}

CollisionResult ResolveAABBPolygon(const SDL_FRect& box, const Polygon& poly)
{
    CollisionResult result;
    SDL_FRect currentBox = box;

    for (const Triangle& tri : poly.triangles)
    {
        vec2D mtv;
        if (TestBoxTriangle(currentBox, tri, mtv))
        {
            result.collided = true;
            result.mtv = result.mtv + mtv;
            currentBox.x += mtv.x;
            currentBox.y += mtv.y;
        }
    }

    return result;
}
