#include "DamageSistem.hpp"

void PlayerStats::lvlUp()
{
    lvlPoint += 6;

    lvl += 1;

    HP += 1;
    defence += 1;
    attackDamage += 1;
    magicDamage += 1;
}

DamageSistem::DamageSistem(Enemy& e, Player& p) :
    enemy(e),
    player(p)
{}

float DamageSistem::LessDamage()
{
    return
        ((player.stats.defence - player.stats.armorPenPercent) - player.stats.armorPen) /
        (player.stats.defence + 350);
}

// float DamageSistem::FinalDamage()
// {

// }
