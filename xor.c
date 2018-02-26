#include <stdio.h>


int main(){
	
/* C:\Users\krystal\Desktop\pokemon\12a_password_13a (09/08/2016 11:20:02)
   StartOffset: 00000002, EndOffset: 0000000E, Length: 0000000D */

unsigned char rawData[13] = {
	0x42, 0xEF, 0xB2, 0x3D, 0xB9, 0x3D, 0x1B, 0x4A, 0x25, 0xBD, 0x25, 0x76,
	0x23
};





unsigned char a = 0x61;
unsigned char b = 0x62;


	
	int i = 0;
	
	while(i<13){
		printf("%02X ", rawData[i] ^ a);
		i++;
	}

	return 0;

}