#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cstddef>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;


struct vec2D { float x = 0.0f, y = 0.0f; };
struct platform {
    std::vector<vec2D> point;
    bool isGround = true;
};

std::vector<platform> platforms;
platform currentPlatform;

vec2D camera { 0.0f, 0.0f };
float zoom { 1.0f };

vec2D mousePos { 0.0f, 0.0f };
vec2D oldMousePos{ 0.0f, 0.0f };
bool isSpacePressed { false };
bool isLeftButtonPressed { false };


// converte le cordinate MONDO a quelle a schermo
// serve per capire dove renderizzare un punto (dato un punto A si trovera a schermo a punto B)
vec2D WorldToScreen(vec2D worldPos)
{
    return {
        worldPos.x - camera.x,
        worldPos.y - camera.y,
    };
}

// il contrario prende una cordinata a schermo e la converte al mondo
// essenzialmente prende appunt un punto dentro lo schermo renzerizzato
// e capisce dove si colloca nel mondo
vec2D ScreenToWorld(vec2D screenPos)
{
    return {
        screenPos.x / zoom + camera.x,
        screenPos.y / zoom + camera.y,
    };
}

// molto semplice calcola la posizione del camera basandosi sullo spostamento del mouse
// e tiene conto anche dello zoom ofc
void MoveCamera()
{
    if(isSpacePressed && isLeftButtonPressed)
    {
        camera.x -= (mousePos.x - oldMousePos.x) / zoom;
        camera.y -= (mousePos.y - oldMousePos.y) / zoom;
    }

    oldMousePos = mousePos;
}

void DrawPoint(SDL_Renderer* renderer, vec2D pos, float radius = 4.0f)
{
    vec2D s = WorldToScreen(pos);
    SDL_FRect point {
        s.x - radius,
        s.y - radius,
        radius * 2.0f,
        radius * 2.0f
    };
    SDL_RenderFillRect(renderer, &point);
}

void DrawPlatform(SDL_Renderer* renderer, const platform& plat, bool closed)
{
    if(plat.point.empty())  return;

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (size_t i = 0; i + 1 < plat.point.size(); i++)
    {
        vec2D a = WorldToScreen({plat.point[i].x, plat.point[i].y});
        vec2D b = WorldToScreen({plat.point[i + 1].x, plat.point[i + 1].y});
        SDL_RenderLine(renderer, a.x, a.y, b.x, b.y);
    }
    if (closed && plat.point.size() > 2)
    {
        vec2D a = WorldToScreen({plat.point.back().x, plat.point.back().y});
        vec2D b = WorldToScreen({plat.point.front().x, plat.point.front().y});
        SDL_RenderLine(renderer, a.x, a.y, b.x, b.y);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(const vec2D &p : plat.point)
    {
        DrawPoint(renderer, p);
    }
}

void SavePlayform(const char* path)
{
    json j = json::array();

    for(const platform& plat : platforms)
    {
        json platJson;
        platJson["isGround"] = plat.isGround;

        json pointsJson = json::array();
        for(const vec2D& p : plat.point)
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
        SDL_Log("could not open the file, ERROR: %s", path);
        return;
    }

    file << j.dump(4);
    file.close();
}

int main()
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("could not init SDL, ERROR: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window { nullptr };
    window = SDL_CreateWindow("level editor", 800, 600, SDL_WINDOW_RESIZABLE);
    if(window == nullptr)
    {
        SDL_Log("could not create the windows, ERROR: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer { nullptr };
    renderer = SDL_CreateRenderer(window, nullptr);
    if(renderer == nullptr)
    {
        SDL_Log("could not create the renderer, ERROR: %s", SDL_GetError());
        return 1;
    }

    if(!SDL_SetRenderVSync(renderer, 1))
        SDL_Log("could not init VSync, ERROR: %s", SDL_GetError());

    SDL_Texture* tex { IMG_LoadTexture(renderer, "assets/map.png") };
    int ScreenWidth { 800 };
    int ScreenHeight { 600 };

    SDL_Event e;
    bool isRunning { true };

    while (isRunning)
    {
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_EVENT_QUIT:
                    isRunning = false;
                break;

                case SDL_EVENT_WINDOW_RESIZED:
                    SDL_GetWindowSizeInPixels(window, &ScreenWidth, &ScreenHeight);
                break;

                case SDL_EVENT_MOUSE_MOTION:
                    mousePos.x = e.motion.x;
                    mousePos.y = e.motion.y;
                break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    switch (e.button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            if(isSpacePressed)
                            {
                                isLeftButtonPressed = true;
                            }
                            else {
                                currentPlatform.point.push_back(ScreenToWorld(mousePos));
                            }
                        break;
                    }
                break;

                case SDL_EVENT_KEY_DOWN:
                    switch (e.key.key)
                    {
                        case SDLK_MINUS:
                            if(zoom > 0.1)
                                zoom -= 0.1;
                        break;

                        case SDLK_PLUS:
                            if(zoom < 3.0)
                                zoom += 0.1;
                        break;

                        case SDLK_SPACE:
                            isSpacePressed = true;
                        break;

                        case SDLK_RETURN:
                            platforms.push_back(currentPlatform);
                            currentPlatform = platform{};
                        break;

                        case SDLK_BACKSPACE:
                            if(!currentPlatform.point.empty())
                            {
                                currentPlatform.point.pop_back();
                            }
                            else if (!platforms.empty())
                            {
                                currentPlatform = platforms.back();
                                platforms.pop_back();
                                if(!currentPlatform.point.empty())
                                {
                                    currentPlatform.point.pop_back();
                                }
                            }
                        break;

                        case SDLK_S:
                            SavePlayform("tools/leveleditor/levelData.json");
                            SDL_Log("platforms saved successfully");
                        break;
                    }
                break;

                case SDL_EVENT_KEY_UP:
                    switch (e.key.key)
                    {
                        case SDLK_SPACE:
                            isSpacePressed = false;
                            isLeftButtonPressed = false;
                        break;
                    }
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        MoveCamera();

        // tutti i calcoli per la camera
        SDL_SetRenderScale(renderer, zoom, zoom);

        if(tex)
        {
            vec2D topLeft { WorldToScreen({ 0.0f, 0.0f }) };
            SDL_FRect rect {
                topLeft.x,
                topLeft.y,
                float(tex->w),
                float(tex->h),
            };
            SDL_RenderTexture(renderer, tex, nullptr, &rect);
        }

        for(auto& p : platforms)
        {
            DrawPlatform(renderer, p, true);
        }

        DrawPlatform(renderer, currentPlatform, false);


        // gui se ma ci sara
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);


        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
