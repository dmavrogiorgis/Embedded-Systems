/*
 * lab4.c
 * Author : Dimitris Mavrogiorgis
 * AM : 2016030016
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 10000000
//#define FOSC 10000000

#define BAUD 9600						//Definition of the desired baud rate
#define UBRR_BAUD F_CPU/16/BAUD-1		//Calculation of UBRR's initial value

uint8_t digit_cnt = 0x00;		//Keeps the digit we display
uint8_t read_char= 0x00;		//Keeps the char we receive

uint8_t data_addr[8];																				//8 numbers to display
uint8_t ok_msg_addr[4] = {0x4F, 0x4B, 0xD, 0xA};													//Here we store ok in memory	
uint8_t seg_dec_addr[11] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF};		//Decoded bcd to 7-seg
	
void TCNT0_INIT();
void USART_INIT(unsigned int ubrr);

extern void MEM_CLEAR();
extern void MEM_INIT();

int main(void){
	cli();						//Turn off interrupts before initialization
	TCNT0_INIT();				//Init the timer0
	USART_INIT(UBRR_BAUD);		//Init the usart
	sei();						//Enable interrupts after initilization
	MEM_INIT();		//Set X point to seg_dec_addr array and Y register point to data_addr array
	MEM_CLEAR();	//Initialize memory with 0x0A in order to turn off the 7-segment display
	
    while (1){
    }
}

void TCNT0_INIT(){

	TCNT0 = 0x63;						// Set TCNT0 the value of 99(0x63) = 255 - (1 ms)*(1 MHz)/64(prescale)
	TCCR0 = (1<<CS01)|(1<<CS00);		//Set prescale 64 - these bits of TCCR0 are set - CS02 = 0  CS01 = 1  CS00 = 1
	TIMSK = (1<<TOIE0);					// TOIE0 is set to 1 in order to enable the timer overflow interrupt
	
	DDRA = 0xFF;		//Set as output PORTA AND PORT C
	DDRC = 0xFF;
	
	PORTA = 0xFF;		//Initialize PORTA with 0xFF which means that 7-seg dispaly is turned off
	PORTC = 0x00;		//Initialize PORTC with 0x00 WHICH mean that none of the digits has enable
	
	return;
}

void USART_INIT(unsigned int ubrr){
	
	UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);		//Enable RXC interrupts, Receive and Transmit
	UBRRH = (unsigned char)(ubrr>>8);		// Set baud rate
	UBRRL = (unsigned char)ubrr;		
	
	UCSRC = (1<<URSEL);					// We make it "1" in order to write to UCSRC
	UCSRC = (1<<UCSZ0)|(1<<UCSZ1);		//8-bit size for everything we transmit and receive
	
	return;
}