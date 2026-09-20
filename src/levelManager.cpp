#include "levelManager.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <unordered_map>
#include <algorithm>

using json = nlohmann::json;
namespace fs = std::filesystem;

// ==================== level ====================

level::level(const char* texture, const char* collision)
{
    map.LoadFromFile(rend.GetRenderer(), texture);
    LoadLevelInfo(collision);
    mapRect = {
        0.0f, // x
        0.0f, // y
        float(map.texture->w),
        float(map.texture->h),
    };
}

level::~level()
{
    map.Destroy();
}

void level::LoadLevelInfo(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        SDL_Log("Impossibile aprire il file di collisione: %s", path);
        return;
    }

    json data;
    try
    {
        file >> data;
    }
    catch (const json::parse_error& e)
    {
        SDL_Log("Errore parsing JSON (%s): %s", path, e.what());
        return;
    }

    for (const auto& platform : data)
    {
        bool isGround = platform.value("isGround", false);
        bool isRect   = platform.value("isRect", false);
        const auto& points = platform["points"];

        if (isRect)
        {
            if (points.size() != 4)
            {
                SDL_Log("Rettangolo con %zu punti invece di 4, saltato", points.size());
                continue;
            }

            Rectangle rect;
            rect.isGround = isGround;
            for (size_t i = 0; i < 4; ++i)
            {
                rect.p[i].x = points[i]["x"].get<float>();
                rect.p[i].y = points[i]["y"].get<float>();
            }
            rects.push_back(rect);
        }
        else
        {
            if (points.size() != 3)
            {
                SDL_Log("Triangolo con %zu punti invece di 3, saltato", points.size());
                continue;
            }

            Triangle tri;
            tri.isGround = isGround;
            for (size_t i = 0; i < 3; ++i)
            {
                tri.p[i].x = points[i]["x"].get<float>();
                tri.p[i].y = points[i]["y"].get<float>();
            }
            tris.push_back(tri);
        }
    }
}

// ==================== levelManager ====================

levelManager::levelManager()
{
    // qua devo inserire il MAPPING di tutti i livelli del gioco (solo path, leggero)
    // (magari piu avanti posso dividere tra inferno overworld e paradiso)

    const std::string textureFolder = "assets";
    const std::string jsonFolder    = "src/levelData";

    // 1. mappa: nome file (senza estensione) -> path completo della texture
    std::unordered_map<std::string, std::string> textureMap;

    try
    {
        for (const fs::directory_entry& entry : fs::directory_iterator(textureFolder))
        {
            if (entry.is_regular_file())
                textureMap[entry.path().stem().string()] = entry.path().string();
        }
    }
    catch (const fs::filesystem_error& e)
    {
        SDL_Log("Errore leggendo la cartella texture: %s", e.what());
        return;
    }

    // 2. itera la cartella dei json, trova il match e registra l'entry (senza caricare nulla)
    try
    {
        for (const fs::directory_entry& entry : fs::directory_iterator(jsonFolder))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".json")
                continue;

            std::string nome = entry.path().stem().string();

            auto it = textureMap.find(nome);
            if (it != textureMap.end())
            {
                entries.push_back(LevelEntry{ nome, it->second, entry.path().string() });
                SDL_Log("Livello trovato: %s", nome.c_str());
            }
            else
            {
                SDL_Log("Attenzione: json '%s' non ha una texture corrispondente", nome.c_str());
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        SDL_Log("Errore leggendo la cartella json: %s", e.what());
        return;
    }

    // ordine deterministico (stanza1, stanza2, stanza3, ...) cosi' l'indice
    // corrisponde a come i livelli si susseguono nel mondo
    std::sort(entries.begin(), entries.end(),
        [](const LevelEntry& a, const LevelEntry& b) { return a.name < b.name; });
}

void levelManager::LoadLevel(int index, Camera& camera)
{
    if (index < 0 || index >= (int)entries.size())
        return;

    const LevelEntry& e = entries[index];

    // costruendo direttamente dentro l'unique_ptr: il vecchio "current" (se c'era)
    // viene distrutto qui, e ~level() libera la sua texture -> questo e' lo "scarico"
    current = std::make_unique<level>(e.texturePath.c_str(), e.jsonPath.c_str());
    current->id = index;
    currentIndex = index;

    camera.SetBounds(float(current->map.texture->w), float(current->map.texture->h));

    SDL_Log("Livello caricato: %s (index %d)", e.name.c_str(), index);
}

void levelManager::Start(int index, Player& player, Camera& camera)
{
    LoadLevel(index, camera);
    if (current)
        player.pos.x = spawnOffset;
}

void levelManager::Update(Player& player, Camera& camera, float dt)
{
    if (!current)
        return;

    // mentre il fade e' in corso, non controllo i bordi: gestisco solo il fade
    if (fadeState != FadeState::None)
    {
        fadeTimer += dt;

        if (fadeState == FadeState::FadingOut)
        {
            if (fadeTimer >= fadeDuration)
            {
                // schermo completamente nero: e' il momento giusto per lo switch vero e proprio
                LoadLevel(pendingIndex, camera);

                if (pendingGoingRight)
                    player.pos.x = spawnOffset; // dentro il nuovo livello, oltre la zona di trigger sinistra
                else
                    player.pos.x = float(current->map.texture->w) - player.width - spawnOffset; // oltre la zona di trigger destra

                fadeState = FadeState::FadingIn;
                fadeTimer = 0.0f;
            }
        }
        else // FadingIn
        {
            if (fadeTimer >= fadeDuration)
            {
                fadeState = FadeState::None;
                fadeTimer = 0.0f;
            }
        }
        return;
    }

    float levelWidth = float(current->map.texture->w);

    // bordo destro raggiunto -> avvia il fade verso il livello successivo (se esiste)
    if (player.pos.x + player.width >= levelWidth - edgeMargin &&
        currentIndex + 1 < (int)entries.size())
    {
        pendingIndex = currentIndex + 1;
        pendingGoingRight = true;
        fadeState = FadeState::FadingOut;
        fadeTimer = 0.0f;
        return;
    }

    // bordo sinistro raggiunto -> avvia il fade verso il livello precedente (se esiste)
    if (player.pos.x <= edgeMargin && currentIndex - 1 >= 0)
    {
        pendingIndex = currentIndex - 1;
        pendingGoingRight = false;
        fadeState = FadeState::FadingOut;
        fadeTimer = 0.0f;
        return;
    }
}

Uint8 levelManager::GetFadeAlpha() const
{
    if (fadeState == FadeState::FadingOut)
    {
        float t = fadeTimer / fadeDuration;
        if (t > 1.0f) t = 1.0f;
        return Uint8(t * 255.0f); // 0 -> 255, sempre piu' nero
    }

    if (fadeState == FadeState::FadingIn)
    {
        float t = fadeTimer / fadeDuration;
        if (t > 1.0f) t = 1.0f;
        return Uint8((1.0f - t) * 255.0f); // 255 -> 0, torna visibile
    }

    return 0;
}

void levelManager::Draw(Camera& camera)
{
    if (!current)
        return;

    current->mapRect.x = -camera.pos.x;
    current->mapRect.y = -camera.pos.y;
    SDL_RenderTexture(rend.GetRenderer(), current->map.texture, nullptr, &current->mapRect);
}
