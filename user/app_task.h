#ifndef BOMB_H
#define BOMB_H

enum BombSignals
{
    A11_SIG = STM_USER_SIG,
    A12_SIG,

    B11_SIG,
    B21_SIG,

    A21_SIG,

    RX_LINE_SIG,

    C1_SIG,

    ACTIVE_OK_SIG,
    A1_SIG,
    B1_SIG,
    C2_SIG,
};

typedef struct
{
    TActive          act;               /* derive from TActive */

} Bomb;


extern Bomb g_bomb;
extern TEvt queue_bomb[3];



void Bomb_ctor(Bomb *me, uint8_t defuse);

#endif

