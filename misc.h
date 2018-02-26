

#ifndef _MISC_H_
#define _MISC_H_

#include <stdio.h>
#include <winsock2.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

typedef enum{
	
	C_PACKET = 1,
	Y_PACKET,
	MSG_PACKET,
	NEWS_PACKET,
	C4_PACKET,
	E_PACKET,
	I_PACKET,
	D_PACKET,
	FRIENDS_PACKET,
	MON_PACKET,
	Q_PACKET,
	NPC_PACKET
	
}PACKET_TYPE;

typedef struct _pPacket{

	PACKET_TYPE type;
	uint8_t *buffer;
	uint32_t size;
	bool alreadySent;
	bool splitSend;
	struct _packet *next;
	
}PACKET, *PPACKET;


enum{
	SUCCESS = 1,
	INVALID_PASSWORD = 2,
	ALREADY_LOGGED = 3,
	OUTDATED_CLIENT = 4,
	BANNED = 5
	
};

typedef enum{
	GREEN = 0,
	RED,
	ORANGE
	
}COLOR;

static const char colors[][20] = {"*GREEN*", "*RED*", "*ORANGE"};

typedef enum _orientation{
	
	NORTH,
	SOUTH,
	EAST,
	WEST
	
}ORIENTATION;

typedef struct _player{

	char username[26];
	char password[26];
	bool isAlreadyConnected;
	uint32_t oldIp;
	
	uint32_t x, y;
	ORIENTATION orientation;
	
	//STATS
	struct {
		
		float rep;
		int fishingLevel;
		
		uint32_t wins, loses, disconnects;
		uint32_t unk3, unk4, unk5;
		
		//registration related
		uint16_t year;
		uint8_t day, month;
		uint8_t hours, minutes, seconds;
		
		//playtime
		uint16_t hoursPlayed, minutesPlayed;
		
		uint8_t star;
		uint8_t sprite;
		uint32_t rndStats1, rndStats2;
		uint32_t unk10;
		
	};
	bool hasGenerated;
	PPACKET packetBuffer;
	struct _player* next;

}PLAYER, *PPLAYER;

static uint8_t packetEnding[] = {0x7C, 0x2E, 0x5C, 0x0D, 0x0A };

//player.c
uint8_t logPlayer(const char* credentials, void *out);
void printPlayerList(PPLAYER pPlayerList);
bool addPlayerToPlayerList(PPLAYER pPlayerList, const char *username, const char *password);

//packet.c
void reversePacket(uint8_t *buffer, uint32_t size);
bool fixPacket(uint8_t *buffer, uint32_t size);
bool analyzePacket(SOCKET s,uint8_t *buffer, uint32_t size);
uint8_t *generateRefPacket(uint8_t refValue);


//encryption
 bool decryptBuffer(uint8_t* buffer, uint32_t size);

#endif