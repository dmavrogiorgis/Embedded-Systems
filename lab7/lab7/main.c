/*
 * lab7.c
 * Author : Dimitris Mavrogiorgis
 * AM : 2016030016
 */ 
#include<avr/io.h>
#include<avr/interrupt.h>
#include <avr/wdt.h>

#define F_CPU 10000000
//#define FOSC 10000000
#define S_HEX 0x53
#define Q_HEX 0x51
#define X_HEX 0x58		// Used to indicate that we have stored a number - we don't care what number
#define O_HEX 0x4F
#define K_HEX 0x4B
#define CR_HEX 0x0D
#define LF_HEX 0x0A

#define TRUE 0x01
#define FALSE 0x00

#define BAUD 9600						//Definition of the desired baud rate
#define UBRR_BAUD F_CPU/16/BAUD-1		//Calculation of UBRR's initial value

register unsigned char input_char asm ("15");

uint8_t prev_instr_char = 0;		//Keeps the char we receive
uint8_t start_or_quit = 0;			//TRUE (1) if start FALSE (0) if quit
uint8_t func_num[3] = {TRUE, FALSE, FALSE};

uint8_t OK_MSG[4] = {0x4F, 0x4B, 0xD, 0xA};		//Here we store ok in memory

uint8_t process_data_1 = 0x00;
uint8_t process_data_2 = 0x00;
uint8_t process_data_3 = 0x00;

uint8_t timer_flag = FALSE;
uint8_t context_switch_flag = FALSE;	

void TCNT0_INIT(){
	TCNT0 = 0x63;						// Set TCNT0 the value of 99(0x63) = 255 - (1 ms)*(1 MHz)/64(prescale)
	TCCR0 = (1<<CS01)|(1<<CS00);		//Set prescale 64 - these bits of TCCR0 are set - CS02 = 0  CS01 = 1  CS00 = 1
	TIMSK |= (1<<TOIE0);					// TOIE1 is set to 1 in order to enable the timer overflow interrupt
	
	DDRA = 0xFF;		//Set as output PORTA AND PORT C
	PORTA = 0x00;		//Initialize PORTC with 0x00 WHICH mean that none of the digits has enable
		
	return;
}

void TCNT1_INIT(){
	TCNT1 = 0xF2F6;		//0xC2F6;
	TCCR1A = 0x00;
	TCCR1B = (1<<CS11)|(1<<CS10);		//// Timer mode with 64 prescler - these bits of TCCR1B are set - CS12 = 0  CS11 = 1  CS10 = 1
	
	TIMSK |= (1 << TOIE1);   // Enable timer1 overflow interrupt(TOIE1)
	
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

void USART_TRANSMIT(){
	
	for(uint8_t i=0; i<4; i++){
		while (!( UCSRA & (1<<UDRE)));
		//UDR = OK_MSG[i];
		TCNT2 = OK_MSG[i];
	}
	prev_instr_char = 0x00;
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

void Process_1(){
	if(timer_flag == TRUE){
		return;
	}else{
		if(PORTA == 0x00){
			PORTA = 0b10101010;
		}else{
			PORTA =~ PORTA;
		}
		timer_flag = TRUE;
	}
	return;
}

void Process_2(){
	if(timer_flag == TRUE){
		return;
	}else{
		if(PORTA == 0x00 || PORTA == 0x80){
			PORTA = 0x01;
		}else{
			PORTA = (PORTA<<1);
		}
		timer_flag = TRUE;
	}
	return;
}

void Process_3(){
	if(timer_flag == TRUE){
		return;
	}else{
		if(PORTA == 0xFF){
			PORTA = 0x00;
		}else{
			PORTA += 0x01;
			timer_flag = TRUE;
		}
	}
	return;
}

int main(void){
	cli();						//Turn off interrupts before initialization
	TCNT0_INIT();				//Init the timer0
	TCNT1_INIT();
	USART_INIT(UBRR_BAUD);		//Init the usart
	sei();						//Enable interrupts after initilization
	
    while (1){
		if(func_num[0] == TRUE){
			Process_1();
			if(context_switch_flag == TRUE){
				func_num[0] = FALSE;	// Disable Process_1
				func_num[1] = TRUE;		// Ånable Process_2
				
				process_data_1 = PORTA;		// Store PORTA to process_data_1
				PORTA = process_data_2;		// Display process_data_2 to PORTA
				context_switch_flag = FALSE;
			}
		}else if(func_num[1] == TRUE){
			Process_2();
			if(context_switch_flag == TRUE){
				func_num[1] = FALSE;	// Disable Process_2
				func_num[2] = TRUE;		// Ånable Process_3
				
				process_data_2 = PORTA;		// Store PORTA to process_data_2
				PORTA = process_data_3;		// Display process_data_3 to PORTA
				context_switch_flag = FALSE;
			}
		}else if(func_num[2] == TRUE){
			
			Process_3();
			if(context_switch_flag == TRUE){
				func_num[2] = FALSE;	// Disable Process_3
				func_num[0] = TRUE;		// Ånable Process_1
				
				process_data_3 = PORTA;		// Store PORTA	to process_data_2
				PORTA = process_data_1;		// Display process_data_1 to PORTA
				context_switch_flag = FALSE;
			}
		}
    }
}

ISR(TIMER1_OVF_vect){
	TCNT1 = 0xF2F6;					// 0xC2F6
	context_switch_flag = TRUE;
}

ISR(TIMER0_OVF_vect){
	TCNT0 = 0x63;
	timer_flag = FALSE;
}

ISR(USART_RXC_vect){
	uint8_t current_char = USART_RECEIVE();
	
	if(prev_instr_char == 0x00 && current_char == S_HEX){
		prev_instr_char = S_HEX;
	}else if(prev_instr_char == 0x00 && current_char == Q_HEX){
		prev_instr_char = Q_HEX;
	}else if(prev_instr_char == S_HEX && (current_char > 0x30 && current_char < 0x34)){
		if(current_char == 0x31){
			func_num[0] = TRUE;
		}else if(current_char == 0x32){
			func_num[1] = TRUE;
		}else{
			func_num[2] = TRUE;
		}
		start_or_quit = TRUE;
		prev_instr_char = X_HEX;
		
	}else if(prev_instr_char == Q_HEX && (current_char > 0x30 && current_char < 0x34)){
		if(current_char == 0x31){
			func_num[0] = FALSE;
			}else if(current_char == 0x32){
			func_num[1] = FALSE;
			}else{
			func_num[2] = FALSE;
		}
		start_or_quit = FALSE;
		prev_instr_char = X_HEX;
		
	}else if(prev_instr_char == X_HEX && current_char == CR_HEX){
		prev_instr_char = CR_HEX;	
			
	}else if(prev_instr_char == CR_HEX && current_char == LF_HEX){
		prev_instr_char = LF_HEX;
		USART_TRANSMIT();
	}else{
		prev_instr_char = 0x00;
	}
}
