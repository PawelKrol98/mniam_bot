#include "amcom_packets.h"

void fillNewGameRequestPayload(const AMCOM_Packet* packet, AMCOM_NewGameRequestPayload* newGameRequest)
{
	memcpy(&newGameRequest->playerNumber, packet->payload, sizeof(newGameRequest->playerNumber));
	memcpy(&newGameRequest->numberOfPlayers, packet->payload + 1, sizeof(newGameRequest->numberOfPlayers));
	memcpy(&newGameRequest->mapWidth, packet->payload + 2, sizeof(newGameRequest->mapWidth));
	memcpy(&newGameRequest->mapHeight, packet->payload + 6, sizeof(newGameRequest->mapHeight));
	LOG_INF("your player_number: %d",  newGameRequest->playerNumber);
	LOG_INF("number of players: %d",  newGameRequest->numberOfPlayers);
	LOG_INF("width: %d",  newGameRequest->mapWidth);
	LOG_INF("height: %d",  newGameRequest->mapHeight);
}

void fillPlayerUpdateRequestPayload(const AMCOM_Packet* packet, AMCOM_PlayerUpdateRequestPayload* playerUpdateRequest)
{
	const uint8_t numberOfFood = packet->header.length / sizeof(AMCOM_PlayerState);
	for (int i = 0; i < numberOfFood; i++)
	{
		memcpy(&playerUpdateRequest->playerState[i].playerNo, packet->payload + i * sizeof(AMCOM_PlayerState), 1);
		memcpy(&playerUpdateRequest->playerState[i].hp, packet->payload + 1 + i * sizeof(AMCOM_PlayerState), 2);
		memcpy(&playerUpdateRequest->playerState[i].x, packet->payload + 3 + i * sizeof(AMCOM_PlayerState), 4);
		memcpy(&playerUpdateRequest->playerState[i].y, packet->payload + 7 + i * sizeof(AMCOM_PlayerState), 4);
		LOG_DBG("Player %d hp:%d x:%f y:%f", playerUpdateRequest->playerState[i].playerNo,
											playerUpdateRequest->playerState[i].hp,
											playerUpdateRequest->playerState[i].x,
											playerUpdateRequest->playerState[i].y);
	}
}

void fillFoodUpdateRequestPayload(const AMCOM_Packet* packet, AMCOM_FoodUpdateRequestPayload* foodUpdateRequest)
{
	const uint8_t numberOfFood = packet->header.length / sizeof(AMCOM_FoodState);
	for (int i = 0; i < numberOfFood; i++)
	{
		memcpy(&foodUpdateRequest->foodState[i].foodNo, packet->payload + i * sizeof(AMCOM_FoodState), 2);
		memcpy(&foodUpdateRequest->foodState[i].state, packet->payload + 2 + i * sizeof(AMCOM_FoodState), 1);
		memcpy(&foodUpdateRequest->foodState[i].x, packet->payload + 3 + i * sizeof(AMCOM_FoodState), 4);
		memcpy(&foodUpdateRequest->foodState[i].y, packet->payload + 7 + i * sizeof(AMCOM_FoodState), 4);
		LOG_DBG("Food %d state:%d x:%f y:%f", foodUpdateRequest->foodState[i].foodNo,
											foodUpdateRequest->foodState[i].state,
											foodUpdateRequest->foodState[i].x,
											foodUpdateRequest->foodState[i].y);
	}
}

void fillMoveRequestPayload(const AMCOM_Packet* packet, AMCOM_MoveRequestPayload* AMCOM_MoveRequestPayload)
{
	memcpy(&AMCOM_MoveRequestPayload->x, packet->payload, 4);
	memcpy(&AMCOM_MoveRequestPayload->y, packet->payload + 4, 4);
	LOG_DBG("current position x:%f y:%f", AMCOM_MoveRequestPayload->x,
										  AMCOM_MoveRequestPayload->y);
}