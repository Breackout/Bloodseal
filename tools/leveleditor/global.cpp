#include <filesystem>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

#include "global.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

// converte le cordinate MONDO a quelle a schermo
// serve per capire dove renderizzare un punto (dato un punto A si trovera a schermo a punto B)
vec2D WorldToScreen(vec2D worldPos, Camera& camera)
{
    return {
        worldPos.x - camera.pos.x,
        worldPos.y - camera.pos.y,
    };
}
// il contrario prende una cordinata a schermo e la converte al mondo
// essenzialmente prende appunt un punto dentro lo schermo renzerizzato
// e capisce dove si colloca nel mondo
vec2D ScreenToWorld(vec2D screenPos, Camera& camera)
{
    return {
        screenPos.x / camera.zoom + camera.pos.x,
        screenPos.y / camera.zoom + camera.pos.y,
    };
}

void Camera::MoveCamera()
{
    if(isSpacePressed && isLeftButtonPressed)
    {
        pos.x -= (mousePos.x - oldMousePos.x) / zoom;
        pos.y -= (mousePos.y - oldMousePos.y) / zoom;
    }

    oldMousePos = mousePos;
}

std::string GetAvailablePath(const std::string& path)
{
    fs::path original(path);

    // Se non esiste ancora, o esiste ma è vuoto, va bene così com'è
    if(!fs::exists(original) || fs::file_size(original) == 0)
        return path;

    std::string stem = original.stem().string();       // nome senza estensione
    std::string ext  = original.extension().string();   // estensione (con il punto)
    fs::path parentDir = original.parent_path();

    int counter = 1;
    fs::path candidate;
    do
    {
        candidate = parentDir / (stem + "_" + std::to_string(counter) + ext);
        counter++;
    }
    while(fs::exists(candidate));

    return candidate.string();
}

void SavePlatform(const char* path)
{
    json j = json::array();

    for(const platform& plat : platforms)
    {
        json platJson;
        platJson["isGround"] = plat.isGround;
        platJson["isRect"] = plat.isRect;

        json pointsJson = json::array();
        for(const vec2D& p : plat.points)
        {
            pointsJson.push_back({
                { "x", p.x },
                { "y", p.y }
            });
        }
        platJson["points"] = pointsJson;

        j.push_back(platJson);
    }

    std::string finalPath = GetAvailablePath(path);

    std::ofstream file(finalPath);
    if(!file.is_open())
    {
        printf("could not open the file , ERROR: %s", finalPath.c_str());
        return;
    }

    file << j.dump(4);
    file.close();

    printf("Salvato in: %s\n", finalPath.c_str());
}
