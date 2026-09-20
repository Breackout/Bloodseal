#pragma once

#include "Render.hpp"
#include "Global.hpp"
#include "Entity.hpp"

#include <vector>
#include <string>
#include <memory>

struct level
{
    Texture map;
    SDL_FRect mapRect;
    int id = -1;

    std::vector<Rectangle> rects;
    std::vector<Triangle> tris;
    std::vector<ChangeLevel> changeLevels; // lista di tutti i posti dove il player puo cambiare livello

    void LoadLevelInfo(const char* path);

    level(const char* texture, const char* collision);
    ~level();

    // la texture SDL non puo' essere copiata in sicurezza (doppio free sulla stessa
    // SDL_Texture*), quindi il level non e' ne' copiabile ne' spostabile: viene
    // sempre gestito tramite std::unique_ptr da levelManager
    level(const level&) = delete;
    level& operator=(const level&) = delete;
    level(level&&) = delete;
    level& operator=(level&&) = delete;
};

// descrive un livello SENZA caricarlo: solo i path. Costa quasi niente tenerne
// tanti in memoria, a differenza di un level vero e proprio (che ha la texture)
struct LevelEntry
{
    std::string name;
    std::string texturePath;
    std::string jsonPath;
};

enum class FadeState
{
    None,
    FadingOut, // schermo che diventa nero (livello vecchio ancora caricato)
    FadingIn   // schermo che torna visibile (livello nuovo gia' caricato)
};

class levelManager
{
    public:
        levelManager(); // scansiona le cartelle, NON carica nessuna texture

        // carica il primo livello di gioco (da chiamare una volta, dopo che
        // player e camera esistono gia')
        void Start(int index, Player& player, Camera& camera);

        // controlla i bordi del livello corrente, gestisce il fade e cambia
        // livello (a meta' del fade) se serve
        void Update(Player& player, Camera& camera, float dt);

        void Draw(Camera& camera);

        level* GetCurrentLevel() { return current.get(); }

        // 0 = schermo normale, 255 = tutto nero. Da usare per disegnare l'overlay
        Uint8 GetFadeAlpha() const;
        bool  IsFading() const { return fadeState != FadeState::None; }

    private:
        std::vector<LevelEntry> entries;   // tutti i livelli conosciuti (solo path)
        std::unique_ptr<level> current;    // l'UNICO livello attualmente caricato
        int currentIndex = -1;

        // margine dal bordo del mondo (in pixel) entro cui scatta il cambio livello
        static constexpr float edgeMargin = 5.0f;

        // distanza dal bordo a cui il player riappare nel nuovo livello. DEVE
        // essere maggiore di edgeMargin, altrimenti il player riappare gia'
        // dentro la zona di trigger e scatena un loop infinito di cambi livello
        static constexpr float spawnOffset = 40.0f;

        // --- fade ---
        FadeState fadeState = FadeState::None;
        float fadeTimer = 0.0f;
        float fadeDuration = 0.5f; // durata di OGNI meta' (out e in), quindi il totale e' il doppio
        int pendingIndex = -1;      // livello verso cui stiamo andando durante il fade
        bool pendingGoingRight = true; // true = prossimo livello, false = precedente

        void LoadLevel(int index, Camera& camera);
};
