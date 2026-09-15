// key binds
// 1 / 2 | cambia tra rettangolo e triangolo
// S | salva in un file
// O | apre il file manager per scegliere un file
// + / - | zooma o dezooma
// cancella | cancella lultima forma che hai fatto
// E | aggiunge nemici
// A | mette un interagibile per cambiare stanza

#include "Editor.hpp"
#include "Render.hpp"
#include "global.hpp"
#include "GUI.hpp"

#include <SDL3_image/SDL_image.h>

Editor::Editor() :
    isRunning(true),
    camera()
{
    base = { 0.0f, 500.0f, float(ScreenWidth), float(ScreenHeight)*0.2f };
    cursorDefault = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    cursorMove = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    currentMode = DrawMode::Rectangle;
    currentStatus = statusBar::base;
}

Editor::~Editor()
{
    if(currentTexture)
        SDL_DestroyTexture(currentTexture);
}

void Editor::OpenImageDialog()
{
    static const SDL_DialogFileFilter filters[] = {
        { "Immagini", "png;jpg;jpeg;bmp" },
        { "Tutti i file", "*" }
    };

    SDL_ShowOpenFileDialog(
        FileDialogCallback,
        this,                    // passiamo 'this' come userdata
        rend.GetWindow(),
        filters,
        SDL_arraysize(filters),
        nullptr,
        false                    // false = un solo file
    );
}

void SDLCALL Editor::FileDialogCallback(void* userdata, const char* const* filelist, int filter)
{
    Editor* self = static_cast<Editor*>(userdata);

    if (!filelist)
    {
        SDL_Log("Errore file dialog: %s", SDL_GetError());
        return;
    }
    if (!*filelist)
        return; // annullato dall'utente

    std::lock_guard<std::mutex> lock(self->fileMutex);
    self->pendingImagePath = filelist[0];
    self->hasNewImage = true;
}

void Editor::LoadImage(const std::string& path)
{
    if (currentTexture)
    {
        SDL_DestroyTexture(currentTexture);
        currentTexture = nullptr;
    }

    currentTexture = IMG_LoadTexture(rend.GetRenderer(), path.c_str());
    if (!currentTexture)
        SDL_Log("Errore caricamento immagine: %s", SDL_GetError());
    else
        SDL_Log("Immagine caricata: %s", path.c_str());
}

vec2D Editor::SnapToNearbyPoint(vec2D worldPos, float radius = 30.0f)
{
    float bestDistSq = radius * radius;
    vec2D bestPoint = worldPos;

    for (const platform& p : platforms)
    {
        for (const vec2D& pt : p.points)
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

void Editor::Update(SDL_Event &e)
{
    base = { 0.0f, float(ScreenHeight) - float(ScreenHeight)*0.2f, float(ScreenWidth), float(ScreenHeight) * 0.2f };
    isHoverUI = mousePos.y >= ScreenHeight *0.2f;

    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_EVENT_QUIT:
                isRunning = false;
            break;

            case SDL_EVENT_WINDOW_RESIZED:
                SDL_GetWindowSizeInPixels(rend.GetWindow(), &ScreenWidth, &ScreenHeight);
            break;

            case SDL_EVENT_MOUSE_MOTION:
                mousePos.x = e.motion.x;
                mousePos.y = e.motion.y;
            break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if(e.button.button == SDL_BUTTON_LEFT)
                {
                    if(isHoverUI)
                    {
                        isLeftButtonPressed = true;
                    }
                    if(isSpacePressed)
                    {
                        isLeftButtonPressed = true;
                    }
                    else
                    {
                        isDrawingShape = true;
                        shapeOrigin = ScreenToWorld(mousePos, camera);
                        shapeOrigin = SnapToNearbyPoint(shapeOrigin);
                    }
                }
            break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if(e.button.button == SDL_BUTTON_LEFT)
                {
                    isLeftButtonPressed = false;
                }
                if (e.button.button == SDL_BUTTON_LEFT && isDrawingShape)
                {
                    vec2D current = ScreenToWorld(mousePos, camera);
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
                            p.points = {
                                { shapeOrigin.x, shapeOrigin.y },
                                { current.x,     shapeOrigin.y },
                                { current.x,     current.y     },
                                { shapeOrigin.x, current.y     }
                            };
                        }
                        else if (currentMode == DrawMode::Triangle)
                        {
                            p.isRect = false;
                            p.points = {
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
                        if(camera.zoom > 0.1)
                            camera.zoom -= 0.1;
                    break;

                    case SDLK_PLUS:
                        if(camera.zoom < 3.0)
                            camera.zoom += 0.1;
                    break;

                    case SDLK_SPACE:
                        isSpacePressed = true;
                        SDL_SetCursor(cursorMove);
                    break;

                    case SDLK_BACKSPACE:
                        if (!platforms.empty())
                            platforms.pop_back();
                    break;

                    case SDLK_S:
                        SavePlatform("tools/leveleditor/collisionData/levelData.json");
                        SDL_Log("platforms saved successfully");
                    break;

                    case SDLK_O:
                        OpenImageDialog();
                    break;
                }
            break;

            case SDL_EVENT_KEY_UP:
                if(e.key.key == SDLK_SPACE)
                {
                    isSpacePressed = false;
                    isLeftButtonPressed = false;
                    SDL_SetCursor(cursorDefault);
                }
            break;
        }
    }

    // Se la callback ha impostato un nuovo file, lo carichiamo qui
    // (siamo sul thread principale, quello giusto per creare texture)
    {
        std::lock_guard<std::mutex> lock(fileMutex);
        if (hasNewImage)
        {
            LoadImage(pendingImagePath);
            hasNewImage = false;
        }
    }

    camera.MoveCamera();
}
void Editor::DrawWorld()
{
    vec2D current = ScreenToWorld(mousePos, camera);

    if(currentTexture)
    {
        vec2D topLeft = { WorldToScreen( {0.0f, 0.0f}, camera) };
        SDL_FRect rect = {
            topLeft.x,
            topLeft.y,
            float(currentTexture->w),
            float(currentTexture->h),
        };
        SDL_RenderTexture(rend.GetRenderer(), currentTexture, nullptr, &rect);
    }

    for(const platform& p : platforms)
    {
        rend.DrawPlatform(camera, p, true);
    }

    if (isDrawingShape)
    {
        platform preview;

        if (currentMode == DrawMode::Rectangle)
        {
            preview.points = {
                { shapeOrigin.x, shapeOrigin.y },
                { current.x,     shapeOrigin.y },
                { current.x,     current.y     },
                { shapeOrigin.x, current.y     }
            };
        }
        else if (currentMode == DrawMode::Triangle)
        {
            preview.points = {
                { shapeOrigin.x, std::min(shapeOrigin.y, current.y) },
                { shapeOrigin.x, std::max(shapeOrigin.y, current.y) },
                { current.x,     std::max(shapeOrigin.y, current.y) }
            };
        }

        rend.DrawPlatform(camera, preview, true); // true = chiude la forma (utile per vedere l'ipotenusa)
    }

    // disegna una previw di dove si trovera il prossimo punto
    if(!isSpacePressed)
    {
        SDL_SetRenderDrawColor(rend.GetRenderer(), 255, 0, 255, 255);
        rend.DrawPoint(camera, current, 4.0f);
    }
}
void Editor::UI()
{
    SDL_SetRenderDrawColor(rend.GetRenderer(), 200, 200, 200, 255);
    SDL_RenderFillRect(rend.GetRenderer(), &base);

    back.Update(isLeftButtonPressed, mousePos, base, [&](){
        currentStatus = statusBar::base;
    });
    back.Draw();

}

void Editor::run()
{
    SDL_Event e;

    while (isRunning)
    {
        // ------ updating stuff -------
                    Update(e);

        // ------- drawing all the world -------
        SDL_SetRenderDrawColor(rend.GetRenderer(), 0, 0, 0, 255);
        SDL_RenderClear(rend.GetRenderer());
        SDL_SetRenderScale(rend.GetRenderer(), camera.zoom, camera.zoom);
                    DrawWorld();

        // ------- drawing the gui -------
        SDL_SetRenderScale(rend.GetRenderer(), 1.0f, 1.0f);
                    UI();

        // ------- end -------
        SDL_RenderPresent(rend.GetRenderer());
    }
}
