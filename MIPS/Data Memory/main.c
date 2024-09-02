#define F_CPU 1000000

//#define RS eS_PORTB2
//#define EN eS_PORTB3
//#define D4 eS_PORTB4
//#define D5 eS_PORTB5
//#define D6 eS_PORTB6
//#define D7 eS_PORTB7

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "lcd.h"

#define memRead 0x00
#define memWrite 0x01
#define aluOutput 0x02

char* align(const char *str) {
	int str_length = strlen(str);
	int num_zeros_to_add = 2 - str_length;

	if (num_zeros_to_add <= 0) {
		// No padding needed, return a copy of the original string
		return strdup(str);
	}

	// Allocate memory for the padded string
	char *padded_str = (char*)malloc(3); // 4 characters + null terminator
	if (!padded_str) {
		// Memory allocation failed
		return NULL;
	}

	// Fill the beginning of the padded string with zeros
	memset(padded_str, '0', num_zeros_to_add);

	// Copy the original string after the padding
	strcpy(padded_str + num_zeros_to_add, str);

	return padded_str;
}

int main(void)
{
	MCUCSR = (1<<JTD);
	MCUCSR = (1<<JTD);
	
	unsigned char memory[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//int aluResult, memoryOp;
	//unsigned char aluResult ,memoryOp ;
	
	
	//char upper[15];
	//char lower[15];
	//char mem[16][3];
	
	DDRA = 0xF0;
	DDRB = 0xFF;
	DDRC = 0xFC;
	DDRD = 0xF0;
	
	PORTB = 0 ;
	
	//Lcd4_Init();
    
    while (1) 
    {
		// constructing command 
		PINC = PINC & 0x03;
		
		// construction aluResult ;
		
		PIND = PIND & 0x0F ;
		
		if(PINC == memRead){
			PORTA = memory[PIND] << 4;
		}
		if(PINC == memWrite){
			memory[PIND] = PINA & 0x0F;
		}
		if(PINC == aluOutput){
			PORTA = PIND << 4;
		}
		
		PORTB = memory[PIND];
		
		
		
		//sprintf(upper, "%s%s%s%s%s%s%s%s", align(itoa(memory[0], mem[0], 16)), align(itoa(memory[1], mem[1], 16)), align(itoa(memory[2], mem[2], 16)), align(itoa(memory[3], mem[3], 16)), align(itoa(memory[4], mem[4], 16)), align(itoa(memory[5], mem[5], 16)), align(itoa(memory[6], mem[6], 16)), align(itoa(memory[7], mem[7], 16)));
		//sprintf(lower, "%s%s%s%s%s%s%s%s", align(itoa(memory[8], mem[8], 16)), align(itoa(memory[9], mem[9], 16)), align(itoa(memory[10], mem[10], 16)), align(itoa(memory[11], mem[11], 16)), align(itoa(memory[12], mem[12], 16)), align(itoa(memory[13], mem[13], 16)), align(itoa(memory[14], mem[14], 16)), align(itoa(memory[15], mem[15], 16)));
		//Lcd4_Set_Cursor(1,1);
		//Lcd4_Write_String(upper);
		//Lcd4_Set_Cursor(2,1);
		//Lcd4_Write_String(lower);
    }
	return 0;
}

