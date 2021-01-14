/*
 * lab4.c
 * Author : Dimitris Mavrogiorgis
 * AM : 2016030016
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define F_CPU 10000000
//#define FOSC 10000000
#define A_HEX 0x41
#define C_HEX 0x43
#define K_HEX 0x4B
#define N_HEX 0x4E
#define O_HEX 0x4F
#define T_HEX 0x54
#define X_HEX 0x58		// Used to indicate that we have stored a number - we don't care what number
#define LF_HEX 0x0A
#define CR_HEX 0x0D
#define R_HEX 0x52

#define BAUD 9600						//Definition of the desired baud rate
#define UBRR_BAUD F_CPU/16/BAUD-1		//Calculation of UBRR's initial value

register unsigned char input_char asm ("15");

uint8_t digit_cnt = 0x00;		//Keeps the digit we display
uint8_t prev_char= 0x00;		//Keeps the char we receive
uint8_t num_of_digits = 0x00;

uint8_t RECEIVED_NUMBERS[8];
uint8_t RESET_MSG[3] = {0x52, 0xD, 0xA};																					//8 numbers to display
uint8_t OK_MSG[4] = {0x4F, 0x4B, 0xD, 0xA};																	//Here we store ok in memory	
uint8_t DECODED_BCD_NUMBERS[11] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF};		//Decoded bcd to 7-seg
	
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

void WDT_INIT(){
	
	if((MCUCSR & (1<<WDRF))){
		MCUCSR &= ~(1<<WDRF);
		for(uint8_t i=0; i<3; i++){
			while (!( UCSRA & (1<<UDRE)));
			//UDR = ok_msg_addr[i];
			TCNT2 = RESET_MSG[i];
		}
	}
	WDTCR |= (1<<WDTOE)|(1<<WDE);
	WDTCR = (1<<WDE)|(1<<WDP1);
}

void MEM_CLEAR(){
	for(uint8_t i=0; i<8; i++){
		RECEIVED_NUMBERS[i] = 0x0A;
	}
	return;
}

void MEM_SHIFT(){
	uint8_t temp;
	for(uint8_t i=7; i>0; i--){
		temp = RECEIVED_NUMBERS[i-1];
		RECEIVED_NUMBERS[i] = temp;
	}
	return;
}

void USART_TRANSMIT(){
	
	for(uint8_t i=0; i<4; i++){
		while (!( UCSRA & (1<<UDRE)));
		
		//UDR = ok_msg_addr[i];
		TCNT2 = OK_MSG[i];
	}
	prev_char = 0x00;
	return;
}

uint8_t USART_RECEIVE(){
	uint8_t received_data;
	
	while (!(UCSRA & (1<<RXC)));
	received_data = UDR;
	received_data = UDR;
	received_data = input_char;
	
	return received_data;
}

int main(void){
	cli();						//Turn off interrupts before initialization
	TCNT0_INIT();				//Init the timer0
	USART_INIT(UBRR_BAUD);		//Init the usart
	WDT_INIT();
	sei();						//Enable interrupts after initilization

	MEM_CLEAR();	//Initialize memory with 0x0A in order to turn off the 7-segment display
	
    while (1){
		
    }
}

ISR(TIMER0_OVF_vect){
	uint8_t display_num;
	
	if(digit_cnt == 0x08){
		digit_cnt = 0x00;
	}
	display_num = RECEIVED_NUMBERS[digit_cnt];
	
	PORTA = 0xFF;
	PORTC = (1<<digit_cnt);
	
	PORTA = DECODED_BCD_NUMBERS[display_num];
	TCNT0 = 0x63;
	digit_cnt++;
	wdt_reset();
}

ISR(USART_RXC_vect){
	uint8_t current_char = USART_RECEIVE();
	
	if(prev_char == 0x00 && current_char == A_HEX){
		prev_char = A_HEX;
	}else if(prev_char == 0x00 && (current_char == C_HEX || current_char == N_HEX)){
		MEM_CLEAR();
		num_of_digits = 0x00;
		prev_char = current_char;
	}else if(prev_char == A_HEX && current_char == T_HEX){
		prev_char = T_HEX;
	}else if(current_char == CR_HEX && (prev_char == T_HEX || prev_char == C_HEX || prev_char == X_HEX)){
		prev_char = CR_HEX;
	}else if((current_char >= 0x30 && current_char <= 0x39) && (prev_char == N_HEX || prev_char == X_HEX)){
		MEM_SHIFT();
		num_of_digits++;
		if(num_of_digits<9){
			RECEIVED_NUMBERS[0] = current_char - 0x30;
			prev_char = X_HEX;
		}
	}else if(prev_char == CR_HEX && current_char == LF_HEX){
		USART_TRANSMIT();
	} else{
		prev_char = 0x00;
		while(1){
		}
	}
	wdt_reset();
}