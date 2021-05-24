#include "mniam_bot.h"
#include <math.h>
#include "logging.h"

float findAngleToGo(struct Position player, struct Position destination)
{
    float deltaX = destination.x - player.x;
    float deltaY = destination.y - player.y;
    float angle = atan(-deltaY / deltaX);
    LOG_DBG("Angle :%f", angle);
    return angle;
}