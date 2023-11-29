#pragma once

enum CLIENT_STATE
{
    ALIVE = 0,
    DEAD
};

struct Message {
    CLIENT_STATE state;
    int num;

    Message(CLIENT_STATE st = ALIVE, int num = 0) : state(st), num(num) {
    }
};
