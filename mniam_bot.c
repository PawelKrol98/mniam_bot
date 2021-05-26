#include "mniam_bot.h"
#include <math.h>
#include "logging.h"

#define PI 3.14159265359

float findAngleToGo(Position player, Position destination)
{
    float deltaX = destination.x - player.x;
    float deltaY = destination.y - player.y;
    float angle= atan2(deltaY, deltaX);
    LOG_DBG("player x:%f y:%f", player.x, player.y);
    LOG_DBG("destination x:%f y:%f", destination.x, destination.y);
    LOG_DBG("Angle :%f", angle);
    return angle;
}

void newGameUpdate(GameInfo* gameInfo, AMCOM_NewGameRequestPayload* newGameRequest)
{
    gameInfo->alivePlayers = newGameRequest->numberOfPlayers;
    gameInfo->ourId = newGameRequest->playerNumber;
}

void playerUpdate(GameInfo* gameInfo, AMCOM_PlayerUpdateRequestPayload* playerUpdateRequest)
{
    uint8_t alivePlayers = 0;
    for (int i = 0; i < AMCOM_MAX_PLAYER_UPDATES; i++)
    {
        gameInfo->players[i].id = playerUpdateRequest->playerState[i].playerNo;
        gameInfo->players[i].hp = playerUpdateRequest->playerState[i].hp;
        gameInfo->players[i].radius = fminf(200, (25 + gameInfo->players[i].hp)/2);
        if (gameInfo->players[i].hp != 0)
        {
            alivePlayers++;
            gameInfo->players[i].position.x = playerUpdateRequest->playerState[i].x;
            gameInfo->players[i].position.y = playerUpdateRequest->playerState[i].y;
        }
    }
    gameInfo->alivePlayers = alivePlayers;
}

void foodUpdate(GameInfo* gameInfo, AMCOM_FoodUpdateRequestPayload* foodUpdateRequest)
{
    uint8_t foodLeft = 0;
    for (int i = 0; i < AMCOM_MAX_FOOD_UPDATES; i++)
    {
        gameInfo->food[i].id = foodUpdateRequest->foodState[i].foodNo;
        gameInfo->food[i].state = foodUpdateRequest->foodState[i].state;
        if (gameInfo->food[i].state)
        {
            foodLeft++;
            gameInfo->food[i].radius = 12.5;
            gameInfo->food[i].position.x = foodUpdateRequest->foodState[i].x;
            gameInfo->food[i].position.y = foodUpdateRequest->foodState[i].y;
        }
    }
    gameInfo->foodLeft = foodLeft;
}

void ourPositionUpdate(GameInfo* gameInfo, AMCOM_MoveRequestPayload* moveRequest)
{
    gameInfo->ourPosition.x = moveRequest->x;
    gameInfo->ourPosition.y = moveRequest->y;
    LOG_DBG("current position x:%f y:%f", gameInfo->ourPosition.x,
										gameInfo->ourPosition.y);
}

void goForTheFirstFood(const AMCOM_FoodUpdateRequestPayload* foodUpdateRequest, const AMCOM_MoveRequestPayload* moveRequest, AMCOM_MoveResponsePayload* moveResponse)
{
    for (int i = 0; i < AMCOM_MAX_FOOD_UPDATES; i ++)
    {
        if (foodUpdateRequest->foodState[i].state != 0)
        {
            struct Position player = {moveRequest->x, moveRequest->y};
            struct Position destination = {foodUpdateRequest->foodState[i].x, foodUpdateRequest->foodState[i].y};
            moveResponse->angle = findAngleToGo(player, destination);
            return;
        }
    }
}