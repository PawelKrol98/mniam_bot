#include "mniam_bot.h"
#include <math.h>
#include "logging.h"

#define PI 3.14159265359

float findAngleToGo(struct Position player, struct Position destination)
{
    float deltaX = destination.x - player.x;
    float deltaY = destination.y - player.y;
    float angle= atan2(deltaY, deltaX);
    LOG_DBG("player x:%f y:%f", player.x, player.y);
    LOG_DBG("destination x:%f y:%f", destination.x, destination.y);
    LOG_DBG("Angle :%f", angle);
    return angle;
}

void goForTheFirstFood(AMCOM_FoodUpdateRequestPayload* foodUpdateRequest, AMCOM_MoveRequestPayload* moveRequest, AMCOM_MoveResponsePayload* moveResponse)
{
    for (int i = 0; i < AMCOM_MAX_FOOD_UPDATES; i ++)
    {
        LOG_DBG("state: %d", foodUpdateRequest->foodState[i].state);
        if (foodUpdateRequest->foodState[i].state != 0)
        {
            struct Position player = {moveRequest->x, moveRequest->y};
            struct Position destination = {foodUpdateRequest->foodState[i].x, foodUpdateRequest->foodState[i].y};
            moveResponse->angle = findAngleToGo(player, destination);
            return;
        }
    }
}