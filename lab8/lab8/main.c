/*
 * lab8.c
 *
 * Created: 26/11/2020 5:44:56 μμ
 * Author : Δημήτρης
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

uint8_t OK_MSG[4] = {0x4F, 0x4B, 0xD, 0xA};		//Here we store OK<CR><LF> in memory

uint8_t process_data_1 = 0x00;
uint8_t process_data_2 = 0x00;
uint8_t process_data_3 = 0x00;

uint8_t timer_flag = FALSE;
uint8_t context_switch_flag = FALSE;

uint8_t current_proc = 0;
uint8_t func_enable = 0x00;
uint8_t func_num[3] = {FALSE, FALSE, FALSE};

void TCNT0_INIT(){
	TCNT0 = 0x63;					// Set TCNT0 the value of 99(0x63) = 255 - (1 ms)*(1 MHz)/64(prescale)
	TCCR0 = (1<<CS01)|(1<<CS00);	//Set prescale 64 - these bits of TCCR0 are set - CS02 = 0  CS01 = 1  CS00 = 1
	TIMSK |= (1<<TOIE0);			// TOIE1 is set to 1 in order to enable the timer overflow interrupt
	
	DDRA = 0xFF;	//Set as output PORTA AND PORT C
	PORTA = 0x00;	//Initialize PORTC with 0x00 WHICH mean that none of the digits has enable
	
	return;
}

void TCNT1_INIT(){
	TCNT1 = 0xC2F6;		//0xF2F6;
	TCCR1A = 0x00;
	TCCR1B = (1<<CS11)|(1<<CS10);	// Timer mode with 64 prescaler - these bits of TCCR1B are set - CS12 = 0  CS11 = 1  CS10 = 1
	
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
			PORTA = 0b10101010; //Initialize first time with tis value
		}else{
			PORTA =~ PORTA;		//Inverse all bits every other time
		}
		timer_flag = TRUE;		//Don't change the output until timer0 makes it false
	}
	return;
}

void Process_2(){
	if(timer_flag == TRUE){
		return;
	}else{
		if(PORTA == 0x00 || PORTA == 0x80){	//Initialize the first time and when "1" reach the 8th bit
			PORTA = 0x01;		
		}else{
			PORTA = (PORTA<<1); //Shift "1" to left 
		}
		timer_flag = TRUE;	//Don't change the output until timer0 makes it false
	}
	return;
}

void Process_3(){
	if(timer_flag == TRUE){
		return;
	}else{
		if(PORTA == 0xFF){	//Implementation of a simple counter that counts from 0x00 to 0xFF
			PORTA = 0x00;
		}else{
			PORTA += 0x01;
			timer_flag = TRUE;	//Don't change the output until timer0 makes it false
		}
	}
	return;
}

void CHECK_ENABLES(){		//Checking if there are changes on function's enables
	if((func_enable & (1<<0))){
		func_num[0] = TRUE;
	}else{
		func_num[0] = FALSE;
		process_data_1 = 0x00;
	}
	
	if((func_enable & (1<<1))){
		func_num[1] = TRUE;
	}else{
		func_num[1] = FALSE;
		process_data_2 = 0x00;
	}
	
	if((func_enable & (1<<2))){
		func_num[2] = TRUE;
	}else{
		func_num[2] = FALSE;
		process_data_3 = 0x00;
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
		if(current_proc == 0){	// The first time we check which function is enabled
			CHECK_ENABLES();
			if(func_num[0] == TRUE){
				current_proc = 1;
			}else if(func_num[1] == TRUE){
				current_proc = 2;
			}else if(func_num[2] == TRUE){
				current_proc = 3;
			}
		}
		if(func_num[0] == TRUE && current_proc == 1){
			Process_1();
			if(context_switch_flag == TRUE){
				CHECK_ENABLES();				//Changing enables
				if(func_num[1] == TRUE){		// Next proccess is No2
					process_data_1 = PORTA;		// Store PORTA to process_data_1
					PORTA = process_data_2;		// Display process_data_2 to PORTA
					current_proc = 2;
					
				}else if(func_num[2] == TRUE && func_num[1] == FALSE){	//Only 1 and 3 are enabled
					process_data_1 = PORTA;								// Store PORTA to process_data_1
					PORTA = process_data_3;								// Display process_data_2 to PORTA
					current_proc = 3;
				}else{
					current_proc = 1;	//Only No1 is enabled
				}
				context_switch_flag = FALSE;
			}
		}else if(func_num[1] == TRUE  && current_proc == 2){
			Process_2();
			if(context_switch_flag == TRUE){
				CHECK_ENABLES();									//Changing enables
				if(func_num[0] == TRUE && func_num[2] == FALSE){	//Only process 1 and 2 are enabled
					process_data_2 = PORTA;							// Store PORTA to process_data_2
					PORTA = process_data_1;							// Display process_data_1 to PORTA
					current_proc = 1;
					
				}else if(func_num[2] == TRUE){	//Next process is No3
					process_data_2 = PORTA;		// Store PORTA to process_data_1
					PORTA = process_data_3;		// Display process_data_3 to PORTA
					current_proc = 3;
				}else{
					current_proc = 2;	//Only No2 is enabled
				}
				context_switch_flag = FALSE;
			}
		}else if(func_num[2] == TRUE  && current_proc == 3){
			Process_3();
			if(context_switch_flag == TRUE){				
				CHECK_ENABLES();									//Changing enables
				if(func_num[1] == TRUE && func_num[0] == FALSE){	//Only process 2 and 3 are enabled
					process_data_3 = PORTA;							// Store PORTA to process_data_2
					PORTA = process_data_2;							// Display process_data_1 to PORTA
					current_proc = 2;
					
				}else if(func_num[0] == TRUE){	//Next process is No1
					process_data_3 = PORTA;		// Store PORTA to process_data_1
					PORTA = process_data_1;		// Display process_data_3 to PORTA
					current_proc = 1;
				}else{
					current_proc = 3;	//Only No1 is enabled
				}
				context_switch_flag = FALSE;
			}
		}
	}
}

ISR(TIMER1_OVF_vect){
	TCNT1 = 0xC2F6;					// 0xF2F6
	context_switch_flag = TRUE;		//This flag indicates that we have to make context switch
}

ISR(TIMER0_OVF_vect){
	TCNT0 = 0x63;
	timer_flag = FALSE;	//This flag indicates that current process has to change PORTA
}

ISR(USART_RXC_vect){
	uint8_t current_char = USART_RECEIVE();
	
	if(prev_instr_char == 0x00 && current_char == S_HEX){
		prev_instr_char = S_HEX;
		}else if(prev_instr_char == 0x00 && current_char == Q_HEX){
		prev_instr_char = Q_HEX;
	}else if(prev_instr_char == S_HEX && (current_char > 0x30 && current_char < 0x34)){
		if(current_char == 0x31){
			func_enable |= (1<<0);	//Set "1" bit 0 of this "register"  
		}else if(current_char == 0x32){
			func_enable |= (1<<1);	//Set "1" bit 1 of this "register"  
		}else{
			func_enable |= (1<<2);	//Set "1" bit 2 of this "register"  
		}
		prev_instr_char = X_HEX;
		
	}else if(prev_instr_char == Q_HEX && (current_char > 0x30 && current_char < 0x34)){
		if(current_char == 0x31){
			func_enable &= ~(1<<0);	//Clear bit 0 of this "register"  
		}else if(current_char == 0x32){
			func_enable &= ~(1<<1);	//Set "1" bit 0 of this "register"  
		}else{
			func_enable &= ~(1<<2);	//Set "1" bit 0 of this "register"  
		}
		prev_instr_char = X_HEX;
		
	}else if(prev_instr_char == X_HEX && current_char == CR_HEX){
		prev_instr_char = CR_HEX;
		
	}else if(prev_instr_char == CR_HEX && current_char == LF_HEX){
		prev_instr_char = LF_HEX;
		USART_TRANSMIT();	//Send OK<CR><LF>
	}else{
		prev_instr_char = 0x00;
	}
}