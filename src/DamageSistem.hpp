#pragma once

#include "Entity.hpp"

// decide come applicare i danni
class DamageSistem
{
    public:
        DamageSistem(Enemy &e, Player& p);

        void Update();
    private:
        Enemy& enemy;
        Player& player;


        float MoltMalus();
        float LessDamage();
        float FinalDamage();
};
