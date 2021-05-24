#ifndef MNIAM_BOT_H_
#define MNIAM_BOT_H_

struct Position
{
    float x;
    float y;
};

float findAngleToGo(struct Position player, struct Position destination);

#endif