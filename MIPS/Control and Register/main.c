#define F_CPU 1000000

#define RS eS_PORTD2
#define EN eS_PORTD3
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "lcd.h"

#define K 0x00
#define A 0x01
#define G 0x02
#define B 0x03
#define L 0x04
#define I 0x05
#define P 0x06
#define J 0x07
#define N 0x08
#define F 0x09
#define M 0x0A
#define D 0x0B
#define C 0x0C
#define H 0x0D
#define E 0x0E
#define O 0x0F

#define zero 0x00
#define t0 0x01
#define t1 0x02
#define t2 0x03
#define t3 0x04
#define t4 0x05
#define sp 0x06
#define temp 0x07

#define add 0x00
#define sub 0x01
#define and 0x02
#define or 0x03
#define nor 0x04
#define sll 0x05
#define srl 0x06

#define beq 0
#define bneq 1
#define jump 2
#define inc 3

#define memRead 0x00
#define memWrite 0x01
#define aluOut 0x02

volatile unsigned char registers[8] = {0,0,0,0,0,0,15,0};
volatile uint16_t instruction, aluOp, branchOp, memoryOp, readRegister1, readRegister2, writeRegister1, immediate, address, jumpAddress;

void UART_init(void){// Normal speed, disable multi-proc
	UCSRA = 0b00000000;// Enable Tx and Rx, disable interrupts
	UCSRB = 0b00011000;// Asynchronous mode, no parity, 1 stop bit, 8 data bits
	UCSRC = 0b10000110;// Baud rate 1200bps, assuming 1MHz clock
	UBRRL = 0x33;
	UBRRH = 0x00;
}

void UART_send(unsigned char data){
	// wait until UDRE flag is set to logic 1
	while ((UCSRA & (1<<UDRE)) == 0x00);
	UDR = data; // Write character to UDR for transmission
}

unsigned char UART_receive(void){
	// Wait until RXC flag is set to logic 1
	while ((UCSRA & (1<<RXC)) == 0x00);
	return UDR; // Read the received character from UDR
}

void SPI_MasterInit(void){
	/* Set MOSI and SCK output, all others input */
	DDRB = (1<<DDB5)|(1<<DDB7);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void SPI_MasterTransmit(char cData){
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
}

uint16_t readRegister(uint16_t regAddress){
	return registers[regAddress];
}

void writeRegister(uint16_t regAddress, uint16_t data){
	registers[regAddress] = data;
}

void rType(uint16_t rr1, uint16_t rr2, uint16_t wr){
	readRegister1 = rr1;
	readRegister2 = rr2;
	writeRegister1 = wr;
	
	PORTB = ((registers[rr1] << 4) & 0xFF) | ((registers[rr2]) & 0xFF);
	//SPI_MasterTransmit(registers[rr2] & 0x0F);
	//SPI_MasterTransmit(registers[rr1] & 0x0F);
}

void sType(uint16_t rr1, uint16_t wr, uint16_t imm){
	readRegister1 = rr1;
	writeRegister1 = wr;
	immediate = imm;
	
	//SPI_MasterTransmit('A');
	PORTB = ((registers[rr1] << 4) & 0xFF) | (imm & 0xFF);
	//SPI_MasterTransmit(((registers[rr1] << 4) & 0xFF) | (imm & 0xFF));
}


void sw(uint16_t rr1, uint16_t rr2, uint16_t imm){
	readRegister1 = rr1;
	readRegister2 = rr2;
	immediate = imm;
	
	PORTB = ((registers[rr1] << 4) & 0xFF) | (imm & 0xFF);
	//SPI_MasterTransmit(imm & 0x0F);	
	//SPI_MasterTransmit(registers[rr1] & 0x0F);	
}

void setAluOp(uint16_t code){
	if(code == A || code == B || code == L || code == M)aluOp = add;		
	if(code == C || code == D || code == N || code == O) aluOp = sub;
	if(code == E || code == F) aluOp = and;
	if(code == G || code == H) aluOp = or;
	if(code == I) aluOp = sll;
	if(code == J) aluOp = srl;
	if(code == K) aluOp = nor;
}

void setBranchOp(uint16_t code, uint16_t jumpAmount){
	if(code == N){ 
		branchOp = beq;
		return;
	}
	if(code == O) {
		branchOp = bneq;
		return;
	}
	if(code == P) {
		branchOp = jump;
		return;
	}	
	branchOp = inc;	
}

void setMemoryOp(uint16_t code){
	if(code == L) memoryOp = memWrite;
	else if(code == M) memoryOp = memRead;
	else memoryOp = aluOut;
}

void setControl(uint16_t inst){
	uint16_t code = inst >> 12;
	uint16_t nibble1 = (inst >> 8) & 0x0F;
	uint16_t nibble2 = (inst >> 4) & 0x0F;
	uint16_t nibble3 = inst & 0x0F;
	jumpAddress = (nibble1 << 4) | nibble2;
	
	setAluOp(code);
	setBranchOp(code, nibble3);
	setMemoryOp(code);
	
	if(code == A || code == C || code == E || code == G || code == K  || code == N || code == O)		
		rType(nibble1, nibble2, nibble3);
	if(code == B || code == D || code == F || code == H ||  code == I || code == J || code == M)
		sType(nibble1, nibble2, nibble3);
	if(code == L) sw(nibble1, nibble2, nibble3);
	
}


char* align(const char *str) {
	int str_length = strlen(str);
	int num_zeros_to_add = 4 - str_length;

	if (num_zeros_to_add <= 0) {
		// No padding needed, return a copy of the original string
		return strdup(str);
	}

	// Allocate memory for the padded string
	char *padded_str = (char*)malloc(5); // 4 characters + null terminator
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
	
	DDRA = 0x0F;
	DDRB = 0xFF;
	DDRC |= 0x7F;
	DDRD |= 0xFC;
	
	UART_init();
	
	//SPI_MasterInit();
	
	Lcd4_Init();
	
	char upper[15];
	char lower[15];
	char reg[6][5];
	uint16_t writeData;
	int opCode;

    while (1) 
    {
		sprintf(upper, "%s %s %s", align(itoa(registers[sp] & 0x0F, reg[zero], 2)), align(itoa(registers[t0] & 0x0F, reg[t0], 2)), align(itoa(registers[t1] & 0x0F, reg[t1], 2)));
		sprintf(lower, "%s %s %s", align(itoa(registers[t2] & 0x0F, reg[t2], 2)), align(itoa(registers[t3] & 0x0F, reg[t3], 2)), align(itoa(registers[t4] & 0x0F, reg[t4], 2)));
		Lcd4_Set_Cursor(1,0);
		Lcd4_Write_String(upper);
		Lcd4_Set_Cursor(2,0);
		Lcd4_Write_String(lower);
		
		instruction = UART_receive();
		instruction |= UART_receive() << 8;
		opCode = instruction >> 12;
		
		setControl(instruction);
				
		PORTC = (branchOp << 5) | (aluOp << 2) | (memoryOp);
		_delay_ms(500);
		
		if(opCode == L) {
			PORTA = readRegister(readRegister2); // prev : 1
		}
		
		if(opCode != L && opCode != N && opCode != O && opCode != P) {
			writeData = (PINA >> 4) & 0x0F;
			writeRegister(writeRegister1, writeData);
		}
				
		
		if(opCode == N) {
			UART_send(instruction & 0x0F);
		}
		else if(opCode == O) {
			UART_send(instruction & 0x0F);
		}
		else if(opCode == P) {
			UART_send(jumpAddress);
		}
		else {
			UART_send(0);
		}
		
		
    }
	
	return 0;
}

