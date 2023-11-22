#pragma once

enum CLIENT_STATE
{
    ALIVE = 0,
    DEAD,
    ALMOST_DEAD,
};

struct Message
{
    unsigned short thrownNumber;
    CLIENT_STATE clientState;
};