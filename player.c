#include "misc.h"

extern PLAYER playerList;
uint8_t checkCredentials(const char *username, uint8_t userSize, const char *password, uint8_t passSize, void *out);



bool addPlayerToPlayerList(PPLAYER pPlayerList, const char *username, const char *password){
	
	
	if(!pPlayerList)//failed for some reason
		return false;
		
	if(pPlayerList->next){
		return addPlayerToPlayerList(pPlayerList->next, username, password);
	}
	else{
		
		strcpy(pPlayerList->username, username);
		strcpy(pPlayerList->password, password);
		
		pPlayerList->rep = 69.0f;
		pPlayerList->fishingLevel = 30;
		pPlayerList->orientation = SOUTH;
		pPlayerList->x = 34;
		pPlayerList->y = 30;
		
		pPlayerList->wins = 1;
		pPlayerList->disconnects = 2;
		pPlayerList->loses = 5;
		pPlayerList->sprite = 3;
		
		pPlayerList->rndStats1 = 8; 
		pPlayerList->rndStats2 = 9;
		
		pPlayerList->year = 2016;
		pPlayerList->day = 12;
		pPlayerList->month = 11;
		
		pPlayerList->hours = 20;
		pPlayerList->minutes = 50;
		pPlayerList->seconds = 10;
		
		pPlayerList->hoursPlayed = 500;
		pPlayerList->minutesPlayed = 40;
		
		pPlayerList->star = 1;
		
		pPlayerList->unk3 = 2;//nop
		pPlayerList->unk4 = 46;
		pPlayerList->unk5 = 0;//nop

		pPlayerList->unk10 = 0;

	/*	float rep;
		int fishingLevel;
		
		int unk, unk1, unk2, unk3, unk4, unk5;
		
		//registration related
		uint16_t year;
		uint8_t day, month;
		uint8_t hours, minutes, seconds;
		
		//playtime
		uint16_t hoursPlayed, minutesPlayed;
		
		int unk6, unk7, unk8, unk9, unk10;*/
		
		pPlayerList->next = malloc(sizeof(PLAYER));
		
		if(!pPlayerList->next)//failed
			return false;
		
		memset(pPlayerList->next, 0, sizeof(PLAYER));
	}
	
	return true;
}


void printPlayerList(PPLAYER pPlayerList){
	
	PPLAYER tmp = pPlayerList;
	uint32_t counter = 1;
	while(tmp->next){
		
		printf("ID: %d \t Username: %s \t Password: %s \n", counter, tmp->username, tmp->password);
		tmp = tmp->next;
		counter++;
	}
}

uint8_t logPlayer(const char* credentials, void* out){
	
	static char *username = NULL;
	static char *password = NULL;
	static char *tmpStr = NULL;
	username = password = tmpStr = NULL;
	
	username = strstr(credentials, "|.|");
	if(!username){
		printf("Error getting player username\n");
		return false;
	}
	
	password = strstr(&username[3], "|.|");
	if(!password){
		printf("Error getting player password\n");
		return false;
	}
	
	tmpStr = strstr(&password[3], "|.|");
	if(!tmpStr){
		
		printf("Error getting password ending\n");
		return false;
	}
	
	
	
	
		
	return checkCredentials(&username[3], (uint8_t)(&password[0] - &username[3]), &password[3], (uint8_t)(&tmpStr[0] - &password[3]), out);
}

uint8_t checkCredentials(const char *username, uint8_t userSize, const char *password, uint8_t passSize, void *out){
	
	static PPLAYER tmp = &playerList;
	
	while(tmp->next){
		
		if(!strncmp(username, tmp->username, (size_t)userSize)){
			//Correct username
			
			if(!strncmp(password, tmp->password, (size_t)passSize)){
				
				if(tmp->isAlreadyConnected)
					return ALREADY_LOGGED;//If already connected then we dont log him
				
				*(uint32_t*)out = (uint32_t)tmp;
				tmp->isAlreadyConnected = true;
				return SUCCESS;
			}
			
			return INVALID_PASSWORD;//Wrong password
			
		}
		tmp = tmp->next;
	}
	return INVALID_PASSWORD;//default
}