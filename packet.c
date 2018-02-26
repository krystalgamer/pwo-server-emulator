#include "misc.h"
#include <windows.h>

void appendPacketEnding(uint8_t *buffer,uint32_t size);
void splitBuffer(uint8_t *buffer,uint32_t size);
void reversePacket(uint8_t *buffer, uint32_t size);


uint8_t *generateConPacket(uint32_t *size);
uint8_t *generateCPackets(uint32_t *size);
uint8_t *generateYPacket(int ip, uint32_t *size);
uint8_t *generateMsgPacket(COLOR color, const char *msg, uint32_t *size);
uint8_t *generateNewsPacket(const char *title, const char *text1, const char *text2, const char *text3, const char *text4, const char *text5, uint32_t *size);
bool generateBigAssPacket(uint8_t *buffer, uint32_t *finalSize, PPLAYER pPlayer);
uint8_t *generateEPacket(uint32_t *size);
uint8_t *generateIPacket(PPLAYER curPlayer, uint32_t *size);
uint8_t *generateDPacket(uint32_t *size);
uint8_t *generateFriendsPacket(uint32_t *size);
uint8_t *generateMonPacket(PPLAYER pPlayer, uint32_t *size);
uint8_t *generateQPacket(PPLAYER pPlayer, uint32_t *size);
uint8_t *generateNPCPacket(uint32_t *size);
bool processAllPackets(uint8_t *buffer, uint32_t size, uint32_t *outSize);
bool analyzeQPacket(char info, PPLAYER pPlayer);
bool analyzeReceivedPacket(char *buffer, uint32_t size, PPLAYER pPlayer);

bool fixPacket(uint8_t *buffer, uint32_t size){
	
	reversePacket(buffer,size-5);
	splitBuffer(buffer, size-5);
	
	return true;
	
}

bool analyzePacket(SOCKET s,uint8_t *buffer, uint32_t size){
	
		char* fixedPacket = buffer;
		
		static PPLAYER playerDestination = NULL;
		uint8_t *packet = NULL;
	
		if(!playerDestination){//means player is not coneceted
			
			if(!decryptBuffer(buffer, size))
				return false;
			if(!fixPacket(buffer, size))
				return false;
			
			if(!strncmp(fixedPacket, "LOG", 3)){
				printf("Login requested detected!\n");
		
				uint8_t logResult = logPlayer(fixedPacket, &playerDestination);
			
				if(logResult >= INVALID_PASSWORD && logResult <= BANNED){
			
					packet = generateRefPacket(logResult);
			
					if(!packet){
						printf("Error creating response packet!.. %d", logResult);
						return false;
					}
					if(send(s, packet,17,0) != 17){
						printf("Couldnt send full packet!");
						return false;
					}
			
					free(packet);
					packet = NULL;
					return true;
				}
				else{//Success
			
					packet = generateConPacket(NULL);
					if(!packet){
						printf("Error creating response packet!.. %d", logResult);
						return false;
					}
					if(send(s, packet,36,0) != 36){
						printf("Couldnt send full packet!");
						return false;
					}
			
					printf("Player logged in successfully!\n");
					packet = NULL;
				
					packet = malloc(4000);
					if(!packet)
						return false;
				
					uint32_t bytesToSend = 0;
				
					while(generateBigAssPacket(packet, &bytesToSend, playerDestination)){
					
						if(!bytesToSend)//The end
							break;
						
						if(send(s, packet,bytesToSend,0) != bytesToSend){
							printf("Couldnt send full packet!");
							return false;
						}
					}
					free(packet);
					packet = NULL;
					return true;
				}
			}
		}
		else{
			
			uint32_t outSize = 0;
			while(processAllPackets(fixedPacket, size, &outSize) || size != 0){
				
				if(!decryptBuffer(fixedPacket, outSize))
					return false;
				
				if(!fixPacket(fixedPacket, outSize))
					return false;
				
				analyzeReceivedPacket(fixedPacket, outSize, playerDestination);
				
				printf("x:%d y:%d\n", playerDestination->x, playerDestination->y);
				fixedPacket = &fixedPacket[outSize];
				size -= outSize;
			}
		}
	
	
	return false;
	
}


bool processAllPackets(uint8_t *buffer, uint32_t size, uint32_t *outSize){
	
	if(!size)
		return false;
	
	uint32_t counter = 0;
	while(counter <= size - 5){
		
		if(!memcmp(&buffer[counter], packetEnding, 5)){
			*outSize = counter + 5;
			return true;
		}
		counter++;
	}
	
	
	return false;
}

//Packet analyze
bool analyzeReceivedPacket(char *buffer, uint32_t size, PPLAYER pPlayer){
	
	if(!size)
		return false;
	
	if(!strncmp(buffer, "q|.|", strlen("q|.|")))
		return analyzeQPacket(buffer[4], pPlayer);
	puts("NOO");
	
	return false;
}


bool analyzeQPacket(char info, PPLAYER pPlayer){
	
	switch(info){
		
		case 'n':
			pPlayer->orientation = NORTH;
			break;
		case 's':
			pPlayer->orientation = SOUTH;
			break;
		case 'e':
			pPlayer->orientation = EAST;
			break;
		case 'w':
			pPlayer->orientation = WEST;
			break;
			
		case 'u':
			pPlayer->y -= (pPlayer->orientation == NORTH) ? 1 : 0;
			break;
		case 'd':
			pPlayer->y += (pPlayer->orientation == SOUTH) ? 1 : 0;
			break;
		case 'l':
			pPlayer->x -= (pPlayer->orientation == WEST) ? 1 : 0;
			break;
		case 'r':
			pPlayer->x += (pPlayer->orientation == EAST) ? 1 : 0;
			break;
		default:
			printf("%c num sei\n", info);
			return false;
	}
	return true;
}
//Packet generation

bool generateBigAssPacket(uint8_t *buffer, uint32_t *finalSize, PPLAYER pPlayer){
	
	if(finalSize)
		*finalSize = 0;
	if(!pPlayer)
		return false;
	
	if(!buffer)
		return false;
	
	//Generate here
	if(!pPlayer->hasGenerated){
		
		
		pPlayer->packetBuffer = malloc(sizeof(PACKET));
		if(!pPlayer->packetBuffer)
			return false;
		
		memset(pPlayer->packetBuffer, 0, sizeof(PACKET));
		
		PPACKET curPacket = pPlayer->packetBuffer;
		PACKET_TYPE curType = C_PACKET;
		
		#define curMaskPacket NPC_PACKET
		while(curType <= curMaskPacket){
			
			curPacket->type = curType;
			
			switch(curPacket->type){
				
				case C_PACKET:
					curPacket->buffer = generateCPackets(&curPacket->size);
					break;
				case Y_PACKET:
					curPacket->buffer = generateYPacket(323456, &curPacket->size);
					break;
				case MSG_PACKET:
					curPacket->buffer = generateMsgPacket(GREEN, "System: OLA FDP DO CRL!", &curPacket->size);
					break;
				case NEWS_PACKET:
					curPacket->buffer = generateNewsPacket("Welcome to PWO v1.97!|", 
					"So... Whats new?||    - Max level now finally is 100|    - Reworked exp formula|    - PCs now have boxes|    - Miscellaneous fixes|||||||||",
					"See forums for a more detailed list of changes|||||",
					"NOTE|If your IP from last login does not match your current one and you|don't have a Dynamic IP - make sure your account is secure with a |password reset and possible staff follow up.|Be sure to check out the website for the latest news!|",
					"You can now follow PWO on twitter! |@damineps| | |Twitter page is monitored by:|[ADM]krystalgamer - @krystalgamer_|[ADM]rJXez - N/A||"
					"Be sure to routinely check the forums for future updates or events!||||", "Thanks for playing PWO|", &curPacket->size);
					break;
				case E_PACKET:
					curPacket->buffer = generateEPacket(&curPacket->size);
					break;
				case C4_PACKET:
					curPacket->buffer = generateCPackets(&curPacket->size);
					curPacket->buffer = strstr(strstr(strstr(curPacket->buffer, "|.\\") + 1 , "|.\\") + 1, "|.\\");//dangerous but idgaf
					
					if(!curPacket->buffer)
						return NULL;
				
					curPacket->buffer += 5;//place it in the right position
					curPacket->size = (uint32_t)((uint32_t)strstr(curPacket->buffer, "|.\\") - (uint32_t)curPacket->buffer) + 5;
					break;
				case I_PACKET:
					curPacket->buffer = generateIPacket(pPlayer, &curPacket->size);
					break;
				case D_PACKET:
					curPacket->buffer = generateDPacket(&curPacket->size);
					break;
				case FRIENDS_PACKET:
					curPacket->buffer = generateFriendsPacket(&curPacket->size);
					break;
				case MON_PACKET:
					curPacket->buffer = generateMonPacket(pPlayer, &curPacket->size);
					break;
				case Q_PACKET:
					curPacket->buffer = generateQPacket(pPlayer, &curPacket->size);
					break;
				case NPC_PACKET:
					curPacket->buffer = generateNPCPacket(&curPacket->size);
					break;
				default:
					goto ignore;
			}
			if(!curPacket->buffer)//Failed for some reason
				return NULL;
			
			if(curType == curMaskPacket){
				pPlayer->hasGenerated = true;
			}
			else{
				curPacket->next = malloc(sizeof(PACKET));
				curPacket = (PPACKET)curPacket->next;
				
				if(!curPacket)
						return false;
				
				memset(curPacket, 0, sizeof(PACKET));
			}
			
			ignore:
			curType++;
		}
		
	}
	
	PPACKET curPacket = pPlayer->packetBuffer;
	
	uint32_t bytesWritten = 0;
	while(curPacket){
		
		if(curPacket->alreadySent){//Dont resent packets
			curPacket = (PPACKET)curPacket->next;
			continue;
		}
		
		
		if((curPacket->size + bytesWritten) <= 4000){
			memcpy(&buffer[bytesWritten], curPacket->buffer, curPacket->size);
			curPacket->alreadySent = true;
			
			bytesWritten += curPacket->size;
			if(finalSize)
				*finalSize = bytesWritten;
		}
		else{
			puts("FODEU!");
			return false;
		}
		
		
		if(curPacket->type == MON_PACKET){//required or the map wont load
			Sleep(50);
			return true;
		}
		curPacket = (PPACKET)curPacket->next;
	}
	
	return true;
}

uint8_t *generateConPacket(uint32_t *size){
	
	if(size)
		*size = 0;
	static char *conPacket = NULL;
	
	#define PWO_MAPS_LINK "Con|.|http://pwo-maps.com/"
	//Already created it no need to do anything
	if(conPacket)
		return conPacket;
	
	//Let's create it babyy
	if(!conPacket){
		conPacket = malloc(strlen(PWO_MAPS_LINK) + 5 * 2);
		if(!conPacket)//failed
			return NULL;
	}
	
	memset(conPacket, 0, strlen(PWO_MAPS_LINK) + 5 * 2);
	//Create it
	strcpy(conPacket, PWO_MAPS_LINK);
	appendPacketEnding(conPacket, strlen(PWO_MAPS_LINK) + 5);
	
	reversePacket((uint8_t*)conPacket, strlen(PWO_MAPS_LINK) + 5);
	splitBuffer(conPacket, strlen(PWO_MAPS_LINK) + 5);
	
	appendPacketEnding(conPacket, strlen(PWO_MAPS_LINK) + 5 *2);
	
	if(!decryptBuffer(conPacket, strlen(PWO_MAPS_LINK) + 5 * 2))
		return NULL;
	
	if(size)
		*size = strlen(PWO_MAPS_LINK) + 5 * 2;
	return conPacket; //Sucess
}

uint8_t *generateRefPacket(uint8_t refValue){
	
	//Ref|.|refValue|.\
	
	char *generatedPacket;
	
	generatedPacket = malloc(17);
	if(!generatedPacket)
		return generatedPacket;
	
	strcpy(generatedPacket, "Ref|.|");
	generatedPacket[6] = '0' + refValue;
	memcpy(&generatedPacket[7], packetEnding, 5);
	
	reversePacket((uint8_t*)generatedPacket, 17);
	
	//Split part
	char tmp[6];
	memcpy(tmp, generatedPacket, 6);
	memcpy(generatedPacket, &generatedPacket[6], 6);
	memcpy(&generatedPacket[6], tmp,6);
	
	//Add packet ending again.. :(
	memcpy(&generatedPacket[12], packetEnding, 5);
	
	if(!decryptBuffer(generatedPacket, 17))
		return NULL;
	
	
	return generatedPacket;
}

uint8_t *generateCPackets(uint32_t *size){
	
	if(size)
		*size = 0;
	#define str1 "C|.|1|1|All"
	#define str2 "C|.|2|1|All|2|Trade"
	#define str3 "C|.|3|1|All|2|Trade|3|Battle"
	#define str4 "C|.|4|1|All|2|Trade|3|Battle|4|Help"
	
	static uint8_t *allCPackets = NULL;
	
	if(allCPackets){
		if(size)
			*size = (strlen(str1) + 5 * 2) + (strlen(str2) + 5 * 2) + (strlen(str3) + 5 * 2) + (strlen(str4) + 5 * 2);
		return allCPackets;
	}
	
	//+5 to the packet ending
	char c1[strlen(str1) + 5 * 2];
	char c2[strlen(str2) + 5 * 2];
	char c3[strlen(str3) + 5 * 2];
	char c4[strlen(str4) + 5 * 2];
	
	memset(c1, 0, strlen(str1) + 5 * 2);
	memset(c2, 0, strlen(str2) + 5 * 2);
	memset(c3, 0, strlen(str3) + 5 * 2);
	memset(c4, 0, strlen(str4) + 5 * 2);
	
	
	if(!c1 || !c2 || !c3 || !c4)//Something failed
		return NULL;
	
	strcpy(c1, str1);
	strcpy(c2, str2);
	strcpy(c3, str3);
	strcpy(c4, str4);
	
	appendPacketEnding(c1, strlen(str1) + 5);
	appendPacketEnding(c1, strlen(str1) + 5 * 2);
	appendPacketEnding(c2, strlen(str2) + 5);
	appendPacketEnding(c2, strlen(str2) + 5 * 2);
	appendPacketEnding(c3, strlen(str3) + 5);
	appendPacketEnding(c3, strlen(str3) + 5 * 2);
	appendPacketEnding(c4, strlen(str4) + 5);
	appendPacketEnding(c4, strlen(str4) + 5 * 2);
	
	
	reversePacket(c1, strlen(str1) + 5);
	reversePacket(c2, strlen(str2) + 5);
	reversePacket(c3, strlen(str3) + 5);
	reversePacket(c4, strlen(str4) + 5);
	
	splitBuffer(c1, strlen(str1) + 5);
	splitBuffer(c2, strlen(str2) + 5);
	splitBuffer(c3, strlen(str3) + 5);
	splitBuffer(c4, strlen(str4) + 5);
	
	
	if(!decryptBuffer(c1, strlen(str1) + 5 * 2))
		return NULL;
	if(!decryptBuffer(c2, strlen(str2) + 5 * 2))
		return NULL;
	if(!decryptBuffer(c3, strlen(str3) + 5 * 2))
		return NULL;
	if(!decryptBuffer(c4, strlen(str4) + 5 * 2))
		return NULL;
	
	
	allCPackets = malloc(strlen(str1) + strlen(str2) + strlen(str3) + strlen(str4) + (4 * (5*2)));
	
	if(!allCPackets)
		return NULL;
	
	memcpy(&allCPackets[0], c1, strlen(str1) + 5 * 2);
	memcpy(&allCPackets[strlen(str1) + 5 * 2], c2, strlen(str2) + 5 * 2);
	memcpy(&allCPackets[(strlen(str1) + 5 * 2) + (strlen(str2) + 5 * 2)], c3, strlen(str3) + 5 * 2);
	memcpy(&allCPackets[(strlen(str1) + 5 * 2) + (strlen(str2) + 5 * 2) + (strlen(str3) + 5 * 2)], c4, strlen(str4) + 5 * 2);
	
	if(size)
		*size = (strlen(str1) + 5 * 2) + (strlen(str2) + 5 * 2) + (strlen(str3) + 5 * 2) + (strlen(str4) + 5 * 2);
	return allCPackets;
}

uint8_t *generateYPacket(int ip, uint32_t *size){
	
	#define yFirstPart "y|.|"
	
	uint8_t ipSize = 0;
	uint8_t currentPart = 0;
	char newIp[16]; //XXX.XXX.XXX.XXX
	char oldIp[16];
	char curIpNumber[4];
	
	memset(newIp, 0, 15);
	
	while(currentPart < 4){
			ipSize += ((((ip>>(8*currentPart)) & 0xFF) >= 100) ? 3 : (((ip>>(8*currentPart)) & 0xFF) >= 10) ? 2 : 1) + ((currentPart != 3) ? 1 : 0);
			
			memset(curIpNumber, 0 , 4);//Clear to re-write
			(currentPart != 3) ? sprintf(curIpNumber,"%d.", (((ip>>(8*currentPart)) & 0xFF))) : sprintf(curIpNumber,"%d", (((ip>>(8*currentPart)) & 0xFF)));
			strcat(newIp, curIpNumber);
			currentPart++;
	}
	uint8_t *tmpIp = malloc(strlen(yFirstPart) + ipSize * 2 + 1 +(2*5));
	if(!tmpIp)
		return NULL;
	memset(tmpIp, 0, strlen(yFirstPart) + ipSize * 2 + 1 +(2*5));//Clears the memory
	
	strcpy(tmpIp, yFirstPart);
	strcpy(&tmpIp[strlen(yFirstPart)], newIp);
	tmpIp[strlen(tmpIp)] = '|';
	strcpy(&tmpIp[strlen(tmpIp)], newIp);
	appendPacketEnding(tmpIp, strlen(yFirstPart) + ipSize * 2 + 1 +(5));
	appendPacketEnding(tmpIp, strlen(yFirstPart) + ipSize * 2 + 1 +(2*5));
	
	//Encryting part
	reversePacket(tmpIp, strlen(yFirstPart) + ipSize * 2 + 1 +(5));
	splitBuffer(tmpIp, strlen(yFirstPart) + ipSize * 2 + 1 +(5));
	if(!decryptBuffer(tmpIp, strlen(yFirstPart) + ipSize * 2 + 1 +(5 * 2))){
		free(tmpIp);
		return NULL;
	}
	
	if(!size){
		free(tmpIp);
		return NULL;
	}
	*size = strlen(yFirstPart) + ipSize * 2 + 1 +(5 * 2);

	return tmpIp;
	
}

uint8_t *generateMsgPacket(COLOR color, const char *msg, uint32_t *size){
	
	#define msgStart "msg|.|"
	if(size)
		*size = 0;
	uint8_t *generatedPacket = NULL;
	
	generatedPacket = malloc(strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5 * 2);
	if(!generatedPacket)
		return NULL;
	
	//Clear it
	memset(generatedPacket, 0, strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5 * 2);
	
	//Create it
	strcpy(generatedPacket, msgStart);
	strcpy(&generatedPacket[strlen(msgStart)], colors[color]);
	strcpy(&generatedPacket[strlen(msgStart)+strlen(colors[color])], msg);
	appendPacketEnding(generatedPacket, strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5);
	appendPacketEnding(generatedPacket, strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5 * 2);
	
	//Encrypt it
	reversePacket(generatedPacket, strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5);
	splitBuffer(generatedPacket, strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5);
	
	if(!decryptBuffer(generatedPacket, strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5 * 2)){
		free(generatedPacket);
		return NULL;
	}
	
	if(size)
		*size = strlen(msgStart) + strlen(colors[color]) + strlen(msg) + 5 * 2;
	return generatedPacket;
	
}

uint8_t *generateNewsPacket(const char *title, const char *text1, const char *text2, const char *text3, const char *text4, const char *text5, uint32_t *size){
	
	#define newsStart "news|.|"
	static uint8_t *newsPacket = NULL;
	static uint32_t fSize = 0;
	
	if(size)
		*size = fSize;
	
	if(newsPacket)
		return newsPacket; //no need to do anything
	
	newsPacket = malloc(strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5 * 2);
	if(!newsPacket)
		return NULL;
	
	memset(newsPacket, 0, strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5 * 2);
	
	strcpy(newsPacket, newsStart);
	strcat(newsPacket, title);
	strcat(newsPacket, text1);
	strcat(newsPacket, text2);
	strcat(newsPacket, text3);
	strcat(newsPacket, text4);
	strcat(newsPacket, text5);
	
	appendPacketEnding(newsPacket, strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5);
	appendPacketEnding(newsPacket, strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5 * 2);
	
	reversePacket(newsPacket, strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5);
	splitBuffer(newsPacket, strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5);
	
	if(!decryptBuffer(newsPacket,  strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5 * 2)){
		free(newsPacket);
		newsPacket = NULL;
		return NULL;
	}
	
	fSize = strlen(newsStart) + strlen(title) + strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4) + strlen(text5) + 5 * 2;
	if(size)
		*size = fSize;
	
	return newsPacket;
	
	
}

uint8_t *generateEPacket(uint32_t *size){
	
	#define eStart "E|.|"
	
	if(size)
		*size = 0;
	
	uint8_t *ePacket = NULL;
	
	ePacket = malloc(strlen(eStart) + 7 + 5 * 2);
	if(!ePacket)
		return NULL;
	
	memset(ePacket, 0, strlen(eStart) + 7 + 5 * 2);
	
	strcpy(ePacket, eStart);
	
	sprintf(&ePacket[strlen(eStart)], "%02d:%02d|%c\0", 6, 23, 'r');
	
	appendPacketEnding(ePacket, strlen(eStart) + 7 + 5);
	
	reversePacket(ePacket, strlen(eStart) + 7 + 5);
	splitBuffer(ePacket, strlen(eStart) + 7 + 5);
	
	appendPacketEnding(ePacket, strlen(eStart) + 7 + 5 * 2);
	
	if(!decryptBuffer(ePacket, strlen(eStart) + 7 + 5 * 2)){
		free(ePacket);
		ePacket = NULL;
		return NULL;
	}
	
	if(size)
		*size = strlen(eStart) + 7 + 5 * 2;
	
	return ePacket;
}

uint8_t *generateIPacket(PPLAYER curPlayer, uint32_t *size){
	
	#define iStart "i|.|"
	char *iPacket = NULL;
	
	//Used to get the size
	static char tmpCounter[0xFF];
	memset(tmpCounter, 0, 0xFF);
	sprintf(tmpCounter, "|%.1f|%d|%d|%d|%d|%d|%d|%d|%d-%02d-%02d %d:%d:%d|%d|%d|%d|%d|%d|%d", curPlayer->rep, curPlayer->fishingLevel,
	curPlayer->wins, curPlayer->loses, curPlayer->disconnects, curPlayer->unk3, curPlayer->unk4, curPlayer->unk5,
	curPlayer->year, curPlayer->day, curPlayer->month, curPlayer->hours, curPlayer->minutes, curPlayer->seconds,
	curPlayer->hoursPlayed, curPlayer->minutesPlayed, curPlayer->star, curPlayer->sprite, curPlayer->rndStats1, curPlayer->rndStats2,
	curPlayer->unk10);
	
	
	if(size)
		*size = 0;
	//todo fix 98 to be dynamic 
	iPacket = malloc(strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5 * 2);
	if(!iPacket)
		return NULL;
	
	memset(iPacket, 0, strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5 * 2);
	
	strcpy(iPacket, iStart);
	strcat(iPacket, curPlayer->username);
	strcat(iPacket, tmpCounter);

	
	appendPacketEnding(iPacket, strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5);
	
	reversePacket(iPacket, strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5);
	splitBuffer(iPacket, strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5);
	
	appendPacketEnding(iPacket, strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5 * 2);
	
	if(!decryptBuffer(iPacket, strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5 * 2)){//failed
		free(iPacket);
		iPacket = NULL;
		return NULL;
	}
	
	if(size)
		*size = strlen(iStart) + strlen(curPlayer->username) + strlen(tmpCounter) + 5 * 2;
	
	return iPacket;
	
}

//TODO make this dynamic
uint8_t *generateDPacket(uint32_t *size){
	
	#define dStart "d|.|"
	#define itemSeparator "\r\n"
	
	char *dPacket = NULL;
	
	if(size)
		*size = 0;
	//todo fix to be dynamic 
	dPacket = malloc(strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5 * 2 );
	if(!dPacket)
		return NULL;
	
	memset(dPacket, 0, strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5 * 2 );
	
	strcpy(dPacket, dStart);
	sprintf(&dPacket[strlen(dStart)], "%d|.|%d|.|%s-=-%d-=-%d-=-%d\r\n%s-=-%d-=-%d-=-%d\r\n%s-=-%d-=-%d-=-%d\r\n", 1664420, 3000,
	"Pokedex", 82, 1, 6, "Pokeball", 4, 5, 5, "Johto Passport", 80, 1, 6);
	
	appendPacketEnding(dPacket, strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5);
	
	reversePacket(dPacket, strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5);
	splitBuffer(dPacket, strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5);
	
	appendPacketEnding(dPacket, strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5 * 2);
	
	if(!decryptBuffer(dPacket, strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5 * 2)){//failed
		free(dPacket);
		dPacket = NULL;
		return NULL;
	}
	
	if(size)
		*size = strlen(dStart) + strlen("1664420") + strlen("3000") + strlen("|.|") * 2 +
	strlen("Pokedex-=-82-=-1-=-6") + 2 + strlen("Pokeball-=-4-=-5-=-5") + 2 + 
	strlen("Johto Passport-=-80-=-1-=-6") + 2 + 5 * 2;
	
	return dPacket;
	
}

uint8_t *generateFriendsPacket(uint32_t *size){
	
	#define friendsStart "Friends|.|"
	static char *friendsPacket;
	
	if(size)
		*size = 0;
	
	if(friendsPacket){
		//TODO
		if(size)
			*size = strlen(friendsStart) + 5 * 2;
		return friendsPacket;
	}
	
	friendsPacket = malloc(strlen(friendsStart) + 5 * 2);
	if(!friendsPacket)
		return NULL;
	
	memset(friendsPacket, 0, strlen(friendsStart) + 5 * 2);
	strcpy(friendsPacket, friendsStart);
	appendPacketEnding(friendsPacket, strlen(friendsStart) + 5);
	
	//TODO
	reversePacket(friendsPacket, strlen(friendsStart) + 5);
	splitBuffer(friendsPacket, strlen(friendsStart) + 5);
	appendPacketEnding(friendsPacket, strlen(friendsStart) + 5 * 2);
	
	if(!decryptBuffer(friendsPacket, strlen(friendsStart) + 5 * 2)){
		free(friendsPacket);
		friendsPacket = NULL;
		return NULL;
	}
	
	if(size)
		*size = strlen(friendsStart) + 5 * 2;
	
	return friendsPacket;
}


//Totally imcomplete
uint8_t *generateMonPacket(PPLAYER pPlayer, uint32_t *size){
	
	#define monStart "mon|.|"
	
	static char *monPacket = NULL;
	
	if(size)
		*size = 0;
	
	if(monPacket){
		if(size)
			*size = strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5 * 2;
		return monPacket;
	}
	
	monPacket = malloc(strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5 * 2);
	if(!monPacket)
		return NULL;
	
	strcpy(monPacket, monStart);
	strcat(monPacket, "26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n");
	
	appendPacketEnding(monPacket, strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5);
	
	appendPacketEnding(monPacket, strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5 * 2);
	
	reversePacket(monPacket, strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5);
	splitBuffer(monPacket, strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5);
	
	if(!decryptBuffer(monPacket, strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5 * 2)){
		free(monPacket);
		monPacket = NULL;
		return NULL;
	}
	
	if(size)
		*size = strlen(monStart) + 
	strlen("26559480|155|1|6|None|22|0|Leer|2|0|0|-1|48|48|Tackle|0|0|50|100|56|56|SmokeScreen|2|0|0|-1|32|32||0|0|0|0|0|0|28|22|5|12|20|31|75|50|163|0|12|10|12|12|14|0|F|Blaze|Hardy|krystalgamer|07.08.2016|NPC|4|0|0|0|0|1|0\r\n") +
	5 * 2;
	
	return monPacket;
}

//Still imcomplete
uint8_t *generateQPacket(PPLAYER pPlayer, uint32_t *size){

	#define qStart "q|.|"
	char *qPacket = NULL;
	
	if(size)
		*size = 0;
	
	qPacket = malloc(strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5 * 2);
	if(!qPacket)
		return NULL;
	
	memset(qPacket, 0, strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5 * 2);
	
	strcpy(qPacket, qStart);
	strcat(qPacket, "51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0");
	appendPacketEnding(qPacket, strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5);
	
	reversePacket(qPacket, strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5);
	splitBuffer(qPacket, strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5);
	
	appendPacketEnding(qPacket, strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5 * 2);
	
	if(!decryptBuffer(qPacket, strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5 * 2)){
		free(qPacket);
		qPacket = NULL;
		return NULL;
	}
	
	if(size)
		*size = strlen(qStart) + strlen("51|.|Cherrygrove City|.|Cherrygrove City|.|34|.|30|.|0|.|0") + 5 * 2;
	
	return qPacket;
}

//Nothing done
uint8_t *generateNPCPacket(uint32_t *size){
	
	static char *npcPacket = NULL;
	
	if(size)
		*size = 0;
	if(npcPacket){
		if(size)
			*size = strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5 * 2;	
		return npcPacket;
	}
	
	npcPacket = malloc(strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5 * 2);
	if(!npcPacket)
		return NULL;
	
	strcpy(npcPacket, "NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\");
	appendPacketEnding(npcPacket, strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5);
	appendPacketEnding(npcPacket, strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5 * 2);
	
	reversePacket(npcPacket, strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5);
	splitBuffer(npcPacket, strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5);
	
	if(!decryptBuffer(npcPacket, strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5 * 2))
	{
		free(npcPacket);
		npcPacket = NULL;
		return NULL;
	}
	
	if(size)
		*size = strlen("NPC|.|Cherrygrove City/.\\53/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Old Man/.\\/.\\Hey there!-=-I'm out of a job now that the Pokecenter is open. Care to battle, sonny!?-=-BATTLE(You are smarter than me...)/.\\Y/.\\73/*\\Cherrygrove City/.\\31/.\\30/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Chef/.\\Jurema/.\\/.\\N/.\\7472/*\\Cherrygrove City/.\\30/.\\31/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7473/*\\Cherrygrove City/.\\32/.\\29/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7474/*\\Cherrygrove City/.\\32/.\\32/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\72/.\\/.\\N/.\\7475/*\\Cherrygrove City/.\\42/.\\30/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\!/.\\/.\\N/.\\7480/*\\Cherrygrove City/.\\46/.\\34/.\\0/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\154/.\\/.\\N/.\\7481/*\\Cherrygrove City/.\\44/.\\35/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\182/.\\/.\\N/.\\7482/*\\Cherrygrove City/.\\46/.\\33/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\OBJECTS/.\\/.\\/.\\N/.\\7483/*\\Cherrygrove City/.\\66/.\\22/.\\3/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\162/.\\/.\\N/.\\7492/*\\Cherrygrove City/.\\28/.\\22/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Officer/.\\Train Station Guard/.\\/.\\N/.\\7501/*\\Cherrygrove City/.\\50/.\\19/.\\2/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\17/.\\/.\\N/.\\7505/*\\Cherrygrove City/.\\35/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Camper/.\\/.\\/.\\N/.\\7507/*\\Cherrygrove City/.\\27/.\\9/.\\1/.\\3/.\\0/.\\1/.\\3/.\\0/.\\0/.\\Pokemon/.\\232/.\\/.\\N/.\\7508/*\\") + 5 * 2;
	
	return npcPacket;
}




//Packet manipulation
void appendPacketEnding(uint8_t *buffer,uint32_t size){
	memcpy(&buffer[size-5], packetEnding, 5);
}

void splitBuffer(uint8_t *buffer,uint32_t size){
	
	uint32_t tmp2 = size/2;
	uint8_t tmp[tmp2];
	
	memcpy(tmp, &buffer[size - tmp2], tmp2);
	memcpy(&buffer[(size - tmp2) - ((tmp2 *2 < size) ? 1 : 0)], &buffer[0], size-tmp2);
	memcpy(&buffer[0], tmp, tmp2);
	
}

void reversePacket(uint8_t *buffer, uint32_t size){
	
	uint8_t tmp = 0;
	uint32_t counter = 0;
	#define realSize (size)
	
	while(counter<(realSize/2)){
		
		tmp = buffer[counter];
		buffer[counter] = buffer[realSize-counter-1];
		buffer[realSize-counter-1] = tmp;
		
		counter++;
	}
	
}


