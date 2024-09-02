/*
 * Demo2.c
 *
 * Created: 2/5/2024 7:09:30 PM
 * Author : Asus
 */ 

#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>
#include <inttypes.h>

#define SS 4
#define MOSI 5
#define MISO 6
#define SCK 7

void SPI_SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDRB = (1<<DDB6);
	/* Enable SPI */
	SPCR = (1<<SPE);
}

char SPI_SlaveReceive(void)
{
	/* Wait for reception complete */
	while(!(SPSR & (1<<SPIF)));
	/* Return data register */
	PORTC = SPDR;
	return PORTC;
}

int main(void)
{
	
	
	DDRA = 0x00;
	DDRB = 0x00;
	DDRC = 0xFF;
	DDRD = 0xFF;
	//SPI_SlaveInit();
	
	unsigned char answer = 0;
	unsigned char isZero =0 ;
	
    while (1){		
		//while((SPSR & (1<<SPIF)) == 0);
		//unsigned char operand = SPI_SlaveReceive();
		unsigned char operand = PINB;
		PORTC = operand;
		
		unsigned char lowerNibble = (operand & 0x0F);
		unsigned char upperNibble = (operand >> 4);
					
		if(PINA == 0b00000000){ // add : 000
			answer = (upperNibble + lowerNibble);
		}
		else if(PINA == 0b00000001){ // sub : 001
			answer = (upperNibble - lowerNibble);
		}
		else if(PINA == 0b00000010){ // and : 010
			answer = (upperNibble & lowerNibble);
		}
		else if(PINA == 0b00000011){ // or : 011
			answer = (upperNibble | lowerNibble);
		}
		else if(PINA == 0b00000100){ // nor : 100
			answer = ~(upperNibble | lowerNibble);
		}
		else if(PINA == 0b00000101){ // sll : 101
			answer = (upperNibble << lowerNibble);
		}
		else if(PINA == 0b00000110){ // srl : 110
			answer = (upperNibble >> lowerNibble);
		}
		
		 isZero = answer==0? 1:0 ;
		 
		answer &= 0x0F ;
		
		answer = (isZero<<4) | answer ;
		
		PORTD = answer ;
		
		//_delay_ms(500);
	}
}

