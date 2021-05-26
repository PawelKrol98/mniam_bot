#ifndef MNIAM_BOT_H_
#define MNIAM_BOT_H_
#include "amcom_packets.h"
#include <stdbool.h>

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
    uint8_t ourId;
    Position ourPosition;
    PlayerInfo players[AMCOM_MAX_PLAYER_UPDATES];
    uint8_t alivePlayers;
    FoodInfo food[AMCOM_MAX_FOOD_UPDATES];
    uint8_t foodLeft;
} GameInfo;

void newGameUpdate(GameInfo*, AMCOM_NewGameRequestPayload*);
void playerUpdate(GameInfo*, AMCOM_PlayerUpdateRequestPayload*);
void foodUpdate(GameInfo*, AMCOM_FoodUpdateRequestPayload*);
void ourPositionUpdate(GameInfo*, AMCOM_MoveRequestPayload*);
float findAngleToGo(Position player, Position destination);
void goForTheFirstFood(const AMCOM_FoodUpdateRequestPayload*,const AMCOM_MoveRequestPayload*, AMCOM_MoveResponsePayload*);

#endif