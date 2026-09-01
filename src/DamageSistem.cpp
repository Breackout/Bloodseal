#include "DamageSistem.hpp"
#include "Entity.hpp"

void PlayerStats::lvlUp()
{
    lvlPoint += 6;

    lvl += 1;

    HP += 1;
    defence += 1;
    attackDamage += 1;
    magicDamage += 1;
}

DamageSistem::DamageSistem(std::vector<Enemy>& e, Player& p) :
    enemis(e),
    player(p)
{}

float DamageSistem::LessDamage(float armorPenPercent, float armorPen)
{
    return
        ((player.stats.defence - armorPenPercent) - armorPen) /
        (player.stats.defence + 350);
}
