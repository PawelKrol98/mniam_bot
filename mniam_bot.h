#ifndef MNIAM_BOT_H_
#define MNIAM_BOT_H_
#include "amcom_packets.h"
#include <stdbool.h>

#define INVALID_PLAYER_ID 255

typedef struct Position
{
    float x;
    float y;
} Position;

typedef struct PlayerInfo
{
    uint8_t id;
    uint16_t hp;
    Position position;
    float direction;
    float radius;
} PlayerInfo;

typedef struct FoodInfo
{
    uint16_t id;
    bool state;
    Position position;
    float radius;
} FoodInfo;

typedef struct GameInfo
{
    float mapHeight;
    float mapWidth;
    uint8_t ourId;
    Position ourPosition;
    PlayerInfo players[AMCOM_MAX_PLAYER_UPDATES];
    uint8_t alivePlayers;
    FoodInfo food[AMCOM_MAX_FOOD_UPDATES];
    uint8_t foodLeft;
} GameInfo;


float findAngleToGo(Position, Position);
float calculateDistance(Position, Position);
void gameInfoInit(GameInfo*);
void newGameUpdate(GameInfo*, AMCOM_NewGameRequestPayload*);
void playerUpdate(GameInfo*, AMCOM_PlayerUpdateRequestPayload*, uint8_t);
void foodUpdate(GameInfo*, AMCOM_FoodUpdateRequestPayload*, uint8_t);
void ourPositionUpdate(GameInfo*, AMCOM_MoveRequestPayload*);
bool killInsteadEat(const GameInfo*, const uint8_t, const uint16_t);
uint16_t findClosestFood(const GameInfo*);
uint8_t findClosestWorsePlayer(const GameInfo*);
uint8_t findClosestPowerfulPlayer(const GameInfo*);
Position findClosestFoodToPlayer(const GameInfo* gameInfo, uint8_t playerId);
float makeDecision(const GameInfo*);

#endif