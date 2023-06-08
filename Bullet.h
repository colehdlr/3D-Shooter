#ifndef FPSGAME_BULLET_H
#define FPSGAME_BULLET_H

#include "raylib.h"


class Bullet {
public:
    Bullet();

    //Variables
    Vector3 startVec;
    Vector3 endVec;
};


#endif //FPSGAME_BULLET_H
