#pragma once

#include "Entity.hpp"
#include <vector>


// tutti i malus che il player puo avere
struct PlayerMalus
{
    bool isPoisoned = false;
    bool isSlowned = false;
    bool isStunned = false;
};




// decide come applicare i danni
class DamageSistem
{
    public:
        DamageSistem(std::vector<Enemy>& e, Player& p);

        void Update();
    private:
        std::vector<Enemy> enemis;
        Player player;


        float MoltMalus();
        float LessDamage(float armorPenPercent, float armorPen);
        float FinalDamage();
};
