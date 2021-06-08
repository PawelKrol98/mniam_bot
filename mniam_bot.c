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

float calculateDistance(Position pointA, Position pointB)
{
    float deltaX = pointA.x - pointB.x;
    float deltaY = pointA.y - pointB.y;
    float square = pow(deltaX, 2) + pow(deltaY, 2);
    return sqrt(square);
}

void newGameUpdate(GameInfo* gameInfo, AMCOM_NewGameRequestPayload* newGameRequest)
{
    gameInfo->alivePlayers = newGameRequest->numberOfPlayers;
    gameInfo->ourId = newGameRequest->playerNumber;
    gameInfo->mapHeight = newGameRequest->mapHeight;
    gameInfo->mapWidth = newGameRequest->mapWidth;
}

void playerUpdate(GameInfo* gameInfo, AMCOM_PlayerUpdateRequestPayload* playerUpdateRequest, uint8_t packetLength)
{
    uint8_t playersInPacket = packetLength / 11;
    for (int i = 0; i < playersInPacket; i++)
    {
        uint8_t playerId = playerUpdateRequest->playerState[i].playerNo;
        gameInfo->players[playerId].id = playerUpdateRequest->playerState[i].playerNo;
        gameInfo->players[playerId].hp = playerUpdateRequest->playerState[i].hp;
        gameInfo->players[playerId].radius = fminf(200, (25 + gameInfo->players[i].hp)/2);
        if (gameInfo->players[playerId].hp != 0)
        {
            gameInfo->players[playerId].position.x = playerUpdateRequest->playerState[i].x;
            gameInfo->players[playerId].position.y = playerUpdateRequest->playerState[i].y;
        }
    }
    uint8_t alivePlayers = 0;
    for (int i = 0; i < AMCOM_MAX_PLAYER_UPDATES; i++)
    {
        if (gameInfo->players[i].hp != 0) alivePlayers++;
    }
    gameInfo->alivePlayers = alivePlayers;
}

void foodUpdate(GameInfo* gameInfo, AMCOM_FoodUpdateRequestPayload* foodUpdateRequest, uint8_t packetLength)
{
    uint8_t foodInPacket = packetLength / 11;
    LOG_INF("Food in packet %d", foodInPacket);
    for (int i = 0; i < foodInPacket; i++)
    {
        uint8_t foodId = foodUpdateRequest->foodState[i].foodNo;
        LOG_INF("Food id: %d", foodId);
        gameInfo->food[foodId].id = foodUpdateRequest->foodState[i].foodNo;
        gameInfo->food[foodId].state = foodUpdateRequest->foodState[i].state;
        if (gameInfo->food[foodId].state)
        {
            gameInfo->food[foodId].radius = 12.5;
            gameInfo->food[foodId].position.x = foodUpdateRequest->foodState[i].x;
            gameInfo->food[foodId].position.y = foodUpdateRequest->foodState[i].y;
            LOG_INF("Food x: %f, food y: %f, food state %d", gameInfo->food[i].position.x, gameInfo->food[i].position.y, gameInfo->food[i].state);
        }
    }
    uint8_t foodLeft = 0;
    for (int i = 0; i < AMCOM_MAX_FOOD_UPDATES; i++)
    {
        if (gameInfo->food[i].state) foodLeft++;
    }
    gameInfo->foodLeft = foodLeft;
    LOG_INF("Food left: %d", gameInfo->foodLeft);
}

void ourPositionUpdate(GameInfo* gameInfo, AMCOM_MoveRequestPayload* moveRequest)
{
    gameInfo->ourPosition.x = moveRequest->x;
    gameInfo->ourPosition.y = moveRequest->y;
    LOG_DBG("current position x:%f y:%f", gameInfo->ourPosition.x,
										gameInfo->ourPosition.y);
}

uint16_t findClosestFood(GameInfo gameInfo)
{
    float closestDistance = sqrt(pow(gameInfo.mapHeight, 2) + pow(gameInfo.mapWidth, 2));
    Position closestFood;
    uint16_t closestFoodId;
    for (int i = 0; i < AMCOM_MAX_FOOD_UPDATES; i++)
    {
        if (gameInfo.food[i].state)
        {
            if (calculateDistance(gameInfo.ourPosition, gameInfo.food[i].position) < closestDistance)
            {
                closestDistance = calculateDistance(gameInfo.ourPosition, gameInfo.food[i].position);
                closestFood = gameInfo.food[i].position;
                closestFoodId = gameInfo.food[i].id;
            }
        }
    }
    return closestFoodId;
}

uint8_t findClosestWorsePlayer(GameInfo gameInfo) {
    
    Position closestEnemy;
    PlayerInfo player = gameInfo.players[gameInfo.ourId];  
    uint8_t closestEnemyId;
    float closestDistance = sqrt(pow(gameInfo.mapHeight, 2) + pow(gameInfo.mapWidth, 2));
    
    for (int i = 0; i < AMCOM_MAX_PLAYER_UPDATES; i++)
    {
        if (player.hp > gameInfo.players[i].hp && gameInfo.players[i].id != gameInfo.ourId)
        {
            if (calculateDistance(gameInfo.ourPosition, gameInfo.players[i].position) < closestDistance)
            {
                closestDistance = calculateDistance(gameInfo.ourPosition, gameInfo.players[i].position);
                closestEnemy = gameInfo.players[i].position;
                closestEnemyId = gameInfo.players[i].id;
            }
        }
    }
    return closestEnemyId;
}

uint8_t findClosestPowerfulPlayer(GameInfo gameInfo) {
    
    Position closestEnemy;
    PlayerInfo player = gameInfo.players[gameInfo.ourId];  
    uint8_t closestEnemyId;
    float closestDistance = sqrt(pow(gameInfo.mapHeight, 2) + pow(gameInfo.mapWidth, 2));
    
    for (int i = 0; i < AMCOM_MAX_PLAYER_UPDATES; i++)
    {
        if (player.hp <= gameInfo.players[i].hp && gameInfo.players[i].id != gameInfo.ourId)
        {
            if (calculateDistance(gameInfo.ourPosition, gameInfo.players[i].position) < closestDistance)
            {
                closestDistance = calculateDistance(gameInfo.ourPosition, gameInfo.players[i].position);
                closestEnemy = gameInfo.players[i].position;
                closestEnemyId = gameInfo.players[i].id;
            }
        }
    }
    return closestEnemyId;
}

uint8_t findClosestPlayerToFood(GameInfo gameInfo, uint16_t foodId) {
    
    Position closestPlayerToFood;
    uint8_t closestPlayerId;
    float closestDistance = sqrt(pow(gameInfo.mapHeight, 2) + pow(gameInfo.mapWidth, 2));
    
    for (int i = 0; i < AMCOM_MAX_PLAYER_UPDATES; i++)
    {
        if(gameInfo.players[i].hp > 0 && gameInfo.players[i].id != gameInfo.ourId) {
            if (calculateDistance(gameInfo.food[foodId].position, gameInfo.players[i].position) < closestDistance)
            {
                closestDistance = calculateDistance(gameInfo.food[foodId].position, gameInfo.players[i].position);
                closestPlayerToFood = gameInfo.players[i].position;
                closestPlayerId = gameInfo.players[i].id;
            }
        }
    }
    return closestPlayerId;
}

float makeDecision(GameInfo gameInfo)
{
    uint16_t closestFoodToEat; 
    uint8_t closestPlayerToEat;

    if (gameInfo.foodLeft > 0) { closestFoodToEat = findClosestFood(gameInfo); }
    //if (gameInfo.alivePlayers > 1) { closestPlayerToEat = findClosestWorstPlayer(gameInfo); }

    // czy przeciwnik nie jest blisko jedzenia
    // jedzenie + najbliższy przeciwnik

    float distanceToFood = calculateDistance(gameInfo.ourPosition, gameInfo.food[closestFoodToEat].position);
    //float distanceToPlayer = calculateDistance(gameInfo.ourPosition, gameInfo.players[closestPlayerToEat].position); 

    if (gameInfo.foodLeft > 0) {
        return findAngleToGo(gameInfo.ourPosition, gameInfo.food[closestFoodToEat].position);
    } else {
        return 0;
    }

    /*
    if (distanceToFood <= distanceToPlayer)
    {
        return findAngleToGo(gameInfo.ourPosition, gameInfo.food[closestFoodToEat].position);
    } else if (distanceToFood > distanceToPlayer) {
        return findAngleToGo(gameInfo.ourPosition, gameInfo.players[closestPlayerToEat].position);
    } else {
        return 0;
    }
    */
   
}