/*
 * MIPS.c
 *
 * Created: 2/5/2024 11:16:36 AM
 * Author : User
 */ 

#define F_CPU 1000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile unsigned char ip ; //instruction pointer

// this section will come from compiler 

uint16_t arr[] = {0b0011000000010101, 0b1011000000101010, 0b1100000000100011, 0b0000000100100011, 0b0100000100100011, 0b1010000101000011, 0b1010000101110011, 0b0100011001110000, 0b1011011001100001, 0b1010000101110011, 0b0100011001110000, 0b1011011001100001, 0b1010000101110011, 0b0100011001110000, 0b1011011001100001, 0b0011011001100001, 0b1010011001010000, 0b0110000101100000, 0b0011000000010001, 0b0001000000010010, 0b1000000100100111, 0b0011010001010001, 0b1111000100100010, 0b0111001000110001, 0b0101001101000010, 0b0000001000110001, 0b1101001000010101, 0b0000001000110101, 0b1101010001001111, 0b0101010001000011, 0b0111010001010011, 0b1110001100000100, 0b1001001100110000, 0b0010010001010011, 0b1000001101011011};
int length = 35;





// this section will come from compiler

void UART_init(void){
	// Normal speed, disable multi-proc
	UCSRA = 0b00000000;
	// Enable Tx and Rx, enable interrupts
	UCSRB = 0b00011000;
	// Asynchronous mode, no parity, 1 stop bit, 8 data bits
	UCSRC = 0b10000110;
	// Baud rate 1200bps, assuming 1MHz clock
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


void handleJump(unsigned char jump){
	
	
	// now I have a jump address , so I'll look for the content of PA3, PA4 , PA5 
	
	// PA3, PA4 constructs the command 
	
	unsigned cmd = (PINA & 0b00011000) >> 3;

 
	
	if(cmd == 3){
		  ip++ ;
		 return ;
		 }  //no action taken
	
	if(cmd == 2){
		// jumping will happen
		
		ip = jump ;
		return ;
	}
	
	//PA5 comes from ALU , zero for no branching, 1 for branching,so let's wait for it
	_delay_ms(300);
	
	if(jump>7){
		jump = jump | 0xF0 ;
	}
	
	unsigned char isZero = (PINA & (1<<PA5)) == 0 ? 0 : 1 ;
	
	if(cmd==0 && isZero){
		// branching will occur 
		//but what will be the amount of jmp? thanks to control, we already have that 
		
		ip += (jump+1) ;   //PC relative addressing 
		
	}else if(cmd==1 && !isZero){
		ip += (jump+1) ;   //PC relative addressing
	} 
	
	PORTC = jump ;
	
	
}




void terminate(){
	while(1);
}


void run_manual(){
	
	if(PINA & (1<<PA2)){  
		
		 while(PINA & (1<<PA2)) ;      //manual mode button has been pressed
		 
		 if(ip>=length) terminate();
		
		 uint16_t instruction = arr[ip] ;
		
	      //pointing to the next address
		 
		
		unsigned char lower_byte = instruction & (0x00FF) ;
		unsigned char upper_byte = instruction >> 8 ;
		
		//now let's send the msg
		
		UART_send(lower_byte) ;
		
		UART_send(upper_byte) ;
		
		//unsigned char cmd = UART_receive();
		
		unsigned char jump = UART_receive();
		
		handleJump(jump);
		
		PORTB = ip;
		
		
	}
	
}

void run_autopilot(){
	
     //running in autopilot mode
	 
	 if(ip>=length) terminate();
	 
	 uint16_t instruction = arr[ip] ;
	 
	 //pointing to the next address
	 
	 
	 unsigned char lower_byte = instruction & (0x00FF) ;
	 unsigned char upper_byte = instruction >> 8 ;
	 
	 //now let's send the msg
	 
	 UART_send(lower_byte) ;
	 
	 UART_send(upper_byte) ;
	 
	 //unsigned char cmd = UART_receive();
	 
	 unsigned char jump = UART_receive();
	 
	 handleJump(jump);
	 
	 PORTB = ip;
	
	_delay_ms(1000);
	
}


int main(void)
{
    /* Replace with your application code */
	
	DDRA = 0b11000000 ; // configuring PA0 as start , PA1 as mode selector , PA2 manual mode button
	ip = 0 ;
	DDRB = 0xFF;
	DDRC = 0xFF;
	
	UART_init();
	
    while (1) 
    {
		if(PINA & (1<<PA0)){
			// program starts
			
			if(PINA & (1<<PA1)){
				run_autopilot();
			}else{
				run_manual();
			}
		}
		
    }

}

