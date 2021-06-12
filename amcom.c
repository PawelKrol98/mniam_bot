#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "amcom.h"

// Start of packet character
const uint8_t  AMCOM_SOP         = 0xA1;
const uint16_t AMCOM_INITIAL_CRC = 0xFFFF;

static uint16_t AMCOM_UpdateCRC(uint8_t byte, uint16_t crc)
{
	byte ^= (uint8_t)(crc & 0x00ff);
	byte ^= (uint8_t)(byte << 4);
	return ((((uint16_t)byte << 8) | (uint8_t)(crc >> 8)) ^ (uint8_t)(byte >> 4) ^ ((uint16_t)byte << 3));
}


void AMCOM_InitReceiver(AMCOM_Receiver* receiver, AMCOM_PacketHandler packetHandlerCallback, void* userContext) {
	receiver->packetHandler = packetHandlerCallback;
	receiver->userContext = userContext;

	receiver->payloadCounter = 0;
	receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
	
	receiver->receivedPacket.header.sop = 0;
	receiver->receivedPacket.header.type = 0;
	receiver->receivedPacket.header.length = 0;
	receiver->receivedPacket.header.crc = AMCOM_INITIAL_CRC;
}

size_t AMCOM_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer) {
	size_t serializedDataSize = 0;
	*destinationBuffer = AMCOM_SOP;
	*(destinationBuffer + 1) = packetType;
	*(destinationBuffer + 2) = payloadSize;
	uint16_t crc = AMCOM_INITIAL_CRC;
	crc = AMCOM_UpdateCRC(packetType, crc);
	crc = AMCOM_UpdateCRC(payloadSize, crc);
	serializedDataSize = 5;
	for(int i = 0; i<payloadSize; i++)
	{
	    if (!(destinationBuffer + serializedDataSize)) break;
	    *(destinationBuffer + serializedDataSize) = *(uint8_t*)(payload + i);
	    crc = AMCOM_UpdateCRC(*(uint8_t*)(payload+i), crc);
	    serializedDataSize++;
	}
	*(destinationBuffer + 3) = crc;
	*(destinationBuffer + 4) = crc>>8;
	return serializedDataSize;
}

void AMCOM_Deserialize(AMCOM_Receiver* receiver, const void* data, size_t dataSize) {
	size_t bytesReceived = 0;
	if (receiver->receivedPacketState < AMCOM_PACKET_STATE_GOT_SOP)
	{
		receiver->receivedPacket.header.sop = *(uint8_t*)data;
		bytesReceived++;
		if (receiver->receivedPacket.header.sop != AMCOM_SOP)
		{
			return;
		}
		receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_SOP;
		if(bytesReceived>=dataSize) return;
	}
	if (receiver->receivedPacketState < AMCOM_PACKET_STATE_GOT_TYPE)
	{
		receiver->receivedPacket.header.type = *(uint8_t*)(data+bytesReceived);
		bytesReceived++;
		receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_TYPE;
		if(bytesReceived>=dataSize) return;
	}
	if (receiver->receivedPacketState < AMCOM_PACKET_STATE_GOT_LENGTH)
	{
		receiver->receivedPacket.header.length = *(uint8_t*)(data+bytesReceived);
		bytesReceived++;
		if(receiver->receivedPacket.header.length > 200){
			receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
			return;
		}
		receiver->receivedPacket.header.crc = *(uint8_t*)(data+bytesReceived);
		bytesReceived++;
		receiver->receivedPacket.header.crc += *(uint8_t*)(data+bytesReceived) <<8;
		bytesReceived++;
		receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_LENGTH;
	}
	if (receiver->receivedPacketState < AMCOM_PACKET_STATE_GOT_WHOLE_PACKET)
	{
		uint8_t bytesReceivedBeforePayload = bytesReceived;
		if(receiver->receivedPacket.header.length > 0)
		{
			receiver->receivedPacketState = AMCOM_PACKET_STATE_GETTING_PAYLOAD;
			for (int i = 0; i < dataSize - bytesReceivedBeforePayload; i++)
			{
				if (!(uint8_t*)(data + bytesReceived)) break;
				receiver->receivedPacket.payload[receiver->payloadCounter] = *(uint8_t*)(data + bytesReceived);
				receiver->payloadCounter++;
				bytesReceived++;
				if (receiver->payloadCounter == receiver->receivedPacket.header.length)
				{
					receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_WHOLE_PACKET;
					receiver->payloadCounter = 0;
					break;
				}
			}
		}
	}
	if(receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_WHOLE_PACKET)
	{
		uint16_t expectedCrc = AMCOM_INITIAL_CRC;
		expectedCrc = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, expectedCrc);
		expectedCrc = AMCOM_UpdateCRC(receiver->receivedPacket.header.length, expectedCrc);
		for(int i = 0; i < receiver->receivedPacket.header.length ; i++ )
		{
			expectedCrc = AMCOM_UpdateCRC(receiver->receivedPacket.payload[i], expectedCrc);
		}
		if(expectedCrc == receiver->receivedPacket.header.crc)
		{
			receiver->packetHandler(&(receiver->receivedPacket), receiver->userContext);
			receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
		}	
		else
		{
			receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
		}
	}
}
