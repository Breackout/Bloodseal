#include <nlohmann/json.hpp>
#include <fstream>

#include "global.hpp"


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

    std::ofstream file(path);
    if(!file.is_open())
    {
        printf("could not open the file , ERROR: %s", path);
        return;
    }

    file << j.dump(4);
    file.close();
}
