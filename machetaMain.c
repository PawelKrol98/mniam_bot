#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "amcom.h"
#include "amcom_packets.h"
#include "logging.h"
#include "mniam_bot.h"

#define DEFAULT_TCP_PORT "2001"

typedef struct {
  GameInfo gameInfo;
  AMCOM_MoveResponsePayload moveResponse;
  SOCKET sock;
} Dane;

/**
 * This function will be called each time a valid AMCOM packet is received
 */
void amcomPacketHandler(const AMCOM_Packet *packet, void *userContext) {
  uint8_t amcomBuf[AMCOM_MAX_PACKET_SIZE]; // buffer used to serialize outgoing
                                           // packets
  size_t bytesToSend = 0;                  // size of the outgoing packet
  static int playerCounter; // just a counter to distinguish player instances
  Dane *ptr = (Dane *)userContext;
  SOCKET sock =
      (SOCKET)ptr->sock; // socket used for communication with the game client
  GameInfo *gameInfo = &(ptr->gameInfo);
  AMCOM_MoveResponsePayload *moveResponse = &(ptr->moveResponse);
  char* names[5] = {"test1", "test2", "test3", "test4", "test5"};
  // static GameInfo gameInfo;
  // static AMCOM_MoveResponsePayload moveResponse;

  switch (packet->header.type) {
  case AMCOM_IDENTIFY_REQUEST:
    LOG_INF("Got IDENTIFY.request. Responding with IDENTIFY.response");
    AMCOM_IdentifyResponsePayload identifyResponse;
    // sprintf(identifyResponse.playerName, "Potezny_uC");
    sprintf(identifyResponse.playerName, names[playerCounter++]);
    bytesToSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse,
                                  sizeof(identifyResponse), amcomBuf);
    break;
  case AMCOM_NEW_GAME_REQUEST:
    LOG_INF("Got NEW_GAME.request.");
    AMCOM_NewGameRequestPayload newGameRequest =
        *(AMCOM_NewGameRequestPayload *)(void *)packet->payload;
    LOG_INF("your player_number: %d", newGameRequest.playerNumber);
    LOG_INF("number of players: %d", newGameRequest.numberOfPlayers);
    LOG_INF("width: %f", newGameRequest.mapWidth);
    LOG_INF("height: %f", newGameRequest.mapHeight);
    newGameUpdate(gameInfo, &newGameRequest);
    bytesToSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, NULL, 0, amcomBuf);
    break;
  case AMCOM_PLAYER_UPDATE_REQUEST:
    LOG_INF("Got PLAYER_UPDATE.request.");
    AMCOM_PlayerUpdateRequestPayload playerUpdateRequest =
        *(AMCOM_PlayerUpdateRequestPayload *)(void *)packet->payload;
    const uint8_t numberOfPlayers =
        packet->header.length / sizeof(AMCOM_PlayerState);
    for (int i = 0; i < numberOfPlayers; i++) {
      LOG_DBG("Player %d hp:%d x:%f y:%f",
              playerUpdateRequest.playerState[i].playerNo,
              playerUpdateRequest.playerState[i].hp,
              playerUpdateRequest.playerState[i].x,
              playerUpdateRequest.playerState[i].y);
    }
    playerUpdate(gameInfo, &playerUpdateRequest, packet->header.length);
    break;
  case AMCOM_FOOD_UPDATE_REQUEST:
    LOG_INF("Got FOOD_UPDATE.request.");
    AMCOM_FoodUpdateRequestPayload foodUpdateRequest =
        *(AMCOM_FoodUpdateRequestPayload *)(void *)packet->payload;
    foodUpdate(gameInfo, &foodUpdateRequest, packet->header.length);
    for (int i = 0; i < AMCOM_MAX_FOOD_UPDATES; i++) {
      LOG_DBG("Food %d state:%d x:%f y:%f", gameInfo.food[i].id,
              gameInfo.food[i].state, gameInfo.food[i].position.x,
              gameInfo.food[i].position.y);
    }
    break;
  case AMCOM_MOVE_REQUEST:
    LOG_DBG("Got MOVE_UPDATE.request.");
    AMCOM_MoveRequestPayload moveRequest =
        *(AMCOM_MoveRequestPayload *)(void *)packet->payload;
    ourPositionUpdate(gameInfo, &moveRequest);
    moveResponse->angle = makeDecision(gameInfo);
    bytesToSend = AMCOM_Serialize(AMCOM_MOVE_RESPONSE, &moveResponse,
                                  sizeof(moveResponse), amcomBuf);
    break;
  }

  if (bytesToSend > 0) {
    int bytesSent = send(sock, (const char *)amcomBuf, bytesToSend, 0);
    if (bytesSent == SOCKET_ERROR) {
      printf("Socket send failed with error: %d", WSAGetLastError());
      closesocket(sock);
      return;
    } else {
      // LOG_INF("Sent %d bytes through socket.", bytesSent);
    }
  }
}

DWORD WINAPI playerThread(LPVOID lpParam) {
  AMCOM_Receiver amcomReceiver; // AMCOM receiver structure

  char buf[512];          // buffer for temporary data
  int receivedBytesCount; // holds the number of bytes received via socket
  Dane *userData = (Dane *)lpParam;
  SOCKET sock = userData->sock;
printf("addr2 %p\n", sock);
  LOG_INF("Got new TCP connection.");

  // Initialize AMCOM receiver
  AMCOM_InitReceiver(&amcomReceiver, amcomPacketHandler, (void *)userData);

  // Receive data from socket until the peer shuts down the connection
  do {
    // Fetch the bytes from socket into buf
    receivedBytesCount = recv(sock, buf, sizeof(buf), 0);
    if (receivedBytesCount > 0) {
      // LOG_INF("Received %d bytes in socket.", receivedBytesCount);
      // Try to deserialize the incoming data
      AMCOM_Deserialize(&amcomReceiver, buf, receivedBytesCount);
    } else if (receivedBytesCount < 0) {
      // Negative result indicates that there was socket communication error
      LOG_ERR("Socket recv failed with error: %d", WSAGetLastError());
      closesocket(sock);
      break;
    }
  } while (receivedBytesCount > 0);

  LOG_INF("Closing connection.");

  // shutdown the connection since we're done
  receivedBytesCount = shutdown(sock, SD_SEND);
  // cleanup
  closesocket(sock);

  return 0;
}

int main(int argc, char **argv) {
  WSADATA wsaData; // socket library data
  SOCKET listenSocket =
      INVALID_SOCKET; // socket on which we will listen for incoming connections
  SOCKET clientSocket =
      INVALID_SOCKET; // socket for actual communication with the game client
  struct addrinfo *addrResult = NULL;
  struct addrinfo hints;
  int result;

  // Say hello
  LOG_INF("mniAM player listening on port %s\nPress CTRL+x to quit",
          DEFAULT_TCP_PORT);

  // Initialize Winsock
  result = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (result != 0) {
    LOG_ERR("WSAStartup failed with error: %d", result);
    return -1;
  }

  // Prepare hints structure
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the server address and port
  result = getaddrinfo(NULL, DEFAULT_TCP_PORT, &hints, &addrResult);
  if (result != 0) {
    LOG_ERR("Function 'getaddrinfo' failed with error: %d", result);
    WSACleanup();
    return -2;
  }

  // Create a socket for connecting to server
  listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype,
                        addrResult->ai_protocol);
  if (listenSocket == INVALID_SOCKET) {
    LOG_ERR("Function 'socket' failed with error: %ld", WSAGetLastError());
    freeaddrinfo(addrResult);
    WSACleanup();
    return -3;
  }
  // Setup the TCP listening socket
  result = bind(listenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
  if (result == SOCKET_ERROR) {
    LOG_ERR("Function 'bind' failed with error: %d", WSAGetLastError());
    freeaddrinfo(addrResult);
    closesocket(listenSocket);
    WSACleanup();
    return -4;
  }
  freeaddrinfo(addrResult);

  // Listen for connections
  result = listen(listenSocket, SOMAXCONN);
  if (result == SOCKET_ERROR) {
    LOG_ERR("Function 'listen' failed with error: %d", WSAGetLastError());
    closesocket(listenSocket);
    WSACleanup();
    return -5;
  }

  while (1) {
    // Accept client socket
    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
      LOG_ERR("Function 'accept' failed with error: %d", WSAGetLastError());
      closesocket(listenSocket);
      WSACleanup();
      return -6;
    } else {
      Dane* userData = malloc(sizeof(*userData));
      userData->sock = clientSocket;
	  printf("addr1 %p\n", userData->sock);
      // Run a separate thread to handle the actual game communication
      CreateThread(NULL, 0, playerThread, (void *)userData, 0, NULL);
    }
    Sleep(10);
  }

  // No longer need server socket
  closesocket(listenSocket);
  // Deinitialize socket library
  WSACleanup();
  // We're done
  return 0;
}