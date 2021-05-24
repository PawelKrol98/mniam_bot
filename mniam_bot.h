#ifndef MNIAM_BOT_H_
#define MNIAM_BOT_H_
#include "amcom_packets.h"

struct Position
{
    float x;
    float y;
};

float findAngleToGo(struct Position player, struct Position destination);

void goForTheFirstFood(AMCOM_FoodUpdateRequestPayload*, AMCOM_MoveRequestPayload*, AMCOM_MoveResponsePayload*);

#endif