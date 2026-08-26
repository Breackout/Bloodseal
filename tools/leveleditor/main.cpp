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
    bool isRect = false;
};

enum class DrawMode { Rectangle, Triangle };
DrawMode currentMode = DrawMode::Rectangle;
bool isDrawingShape { false };
vec2D shapeOrigin { 0.0f, 0.0f };

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

vec2D SnapToNearbyPoint(vec2D worldPos, float radius = 30.0f)
{
    float bestDistSq = radius * radius;
    vec2D bestPoint = worldPos;

    for (const platform& p : platforms)
    {
        for (const vec2D& pt : p.point)
        {
            float dx = pt.x - worldPos.x;
            float dy = pt.y - worldPos.y;
            float distSq = dx * dx + dy * dy;

            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                bestPoint = pt;
            }
        }
    }

    return bestPoint;
}

// solo dio e claude code sa come funziona questa funzione pero funziona
void DrawThickLine(SDL_Renderer* renderer, vec2D a, vec2D b, SDL_Color color, float screenThickness = 2.0f)
{
    vec2D sa = WorldToScreen(a);
    vec2D sb = WorldToScreen(b);

    float dx = sb.x - sa.x;
    float dy = sb.y - sa.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) return;

    // normale perpendicolare alla linea, normalizzata
    float nx = -dy / len;
    float ny = dx / len;

    // spessore costante a schermo indipendentemente dallo zoom
    float half = (screenThickness / zoom) * 0.5f;

    SDL_FColor c { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    SDL_Vertex verts[4] = {
        { { sa.x + nx * half, sa.y + ny * half }, c, {0, 0} },
        { { sa.x - nx * half, sa.y - ny * half }, c, {0, 0} },
        { { sb.x - nx * half, sb.y - ny * half }, c, {0, 0} },
        { { sb.x + nx * half, sb.y + ny * half }, c, {0, 0} },
    };

    int indices[6] = { 0, 1, 2, 0, 2, 3 };

    SDL_RenderGeometry(renderer, nullptr, verts, 4, indices, 6);
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

void DrawPoint(SDL_Renderer* renderer, vec2D pos, float baseRadius = 4.0f)
{
    vec2D s = WorldToScreen(pos);

    // Dividiamo per zoom così la dimensione a schermo rimane costante
    float currentRadius = baseRadius / zoom;

    SDL_FRect point {
        s.x - currentRadius,
        s.y - currentRadius,
        currentRadius * 2.0f,
        currentRadius * 2.0f
    };
    SDL_RenderFillRect(renderer, &point);
}

void DrawPlatform(SDL_Renderer* renderer, const platform& plat, bool closed)
{
    if(plat.point.empty()) return;

    SDL_Color lineColor { 0, 255, 0, 255 };
    for (size_t i = 0; i + 1 < plat.point.size(); i++)
    {
        DrawThickLine(renderer, plat.point[i], plat.point[i + 1], lineColor);
    }

    if (closed && plat.point.size() > 2)
    {
        DrawThickLine(renderer, plat.point.back(), plat.point.front(), lineColor);
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
        platJson["isRect"] = plat.isRect;

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

    SDL_Cursor* cursorDefault = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    SDL_Cursor* cursorMove = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);

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
                            else
                            {
                                isDrawingShape = true;
                                shapeOrigin = ScreenToWorld(mousePos);
                                shapeOrigin = SnapToNearbyPoint(shapeOrigin);
                            }
                        break;
                    }
                break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (e.button.button == SDL_BUTTON_LEFT && isDrawingShape)
                    {
                        vec2D current = ScreenToWorld(mousePos);
                        current = SnapToNearbyPoint(current);

                        // evita di creare forme degeneri se il drag è troppo corto
                        float dx = std::abs(current.x - shapeOrigin.x);
                        float dy = std::abs(current.y - shapeOrigin.y);

                        if (dx > 2.0f && dy > 2.0f)
                        {
                            platform p;
                            p.isGround = true;

                            if (currentMode == DrawMode::Rectangle)
                            {
                                p.isRect = true;
                                p.point = {
                                    { shapeOrigin.x, shapeOrigin.y },
                                    { current.x,     shapeOrigin.y },
                                    { current.x,     current.y     },
                                    { shapeOrigin.x, current.y     }
                                };
                            }
                            else if (currentMode == DrawMode::Triangle)
                            {
                                p.isRect = false;
                                p.point = {
                                    { shapeOrigin.x, std::min(shapeOrigin.y, current.y) }, // vertice in alto (apice)
                                    { shapeOrigin.x, std::max(shapeOrigin.y, current.y) }, // angolo retto, stessa x dell'apice
                                    { current.x,     std::max(shapeOrigin.y, current.y) }  // base, stessa y dell'angolo retto
                                };
                            }

                            platforms.push_back(p);
                        }

                        isDrawingShape = false;
                    }
                break;

                case SDL_EVENT_KEY_DOWN:
                    switch (e.key.key)
                    {
                        case SDLK_1:
                            currentMode = DrawMode::Rectangle;
                        break;

                        case SDLK_2:
                            currentMode = DrawMode::Triangle;
                        break;

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
                            SDL_SetCursor(cursorMove);
                        break;

                        case SDLK_RETURN:
                            platforms.push_back(currentPlatform);
                            currentPlatform = platform{};
                        break;

                        case SDLK_BACKSPACE:
                            if (!platforms.empty())
                                platforms.pop_back();
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
                            SDL_SetCursor(cursorDefault);
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
        vec2D current = ScreenToWorld(mousePos);

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

        if (isDrawingShape)
        {
            platform preview;

            if (currentMode == DrawMode::Rectangle)
            {
                preview.point = {
                    { shapeOrigin.x, shapeOrigin.y },
                    { current.x,     shapeOrigin.y },
                    { current.x,     current.y     },
                    { shapeOrigin.x, current.y     }
                };
            }
            else if (currentMode == DrawMode::Triangle)
            {
                preview.point = {
                    { shapeOrigin.x, std::min(shapeOrigin.y, current.y) },
                    { shapeOrigin.x, std::max(shapeOrigin.y, current.y) },
                    { current.x,     std::max(shapeOrigin.y, current.y) }
                };
            }

            DrawPlatform(renderer, preview, true); // true = chiude la forma (utile per vedere l'ipotenusa)
        }

        // disegna una previw di dove si trovera il prossimo punto
        if(!isSpacePressed)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
            DrawPoint(renderer, current);
        }



        // gui se ma ci sara
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);


        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
