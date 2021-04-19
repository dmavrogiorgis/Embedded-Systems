/*
 * lab9.c
 *
 * Created: 11/12/2020 4:53:48 μμ
 * Author : Δημήτρης
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define is_bit_set(byte,bit)	(byte & (1 << bit))
#define is_bit_clear(byte,bit)	(!(byte & (1 << bit)))
#define set_bit(byte,bit)	((byte) |=  (1<<(bit)))
#define clear_bit(byte,bit)	((byte) &= ~(1<<(bit)))
#define flip_bit(byte,bit)  ((byte) ^=  (1<<(bit)))

#define TRUE 0x01
#define FALSE 0x00

uint8_t count_changes = 0;
uint8_t out_val = 0;

/* The 2 pins of SPDT switch */
uint8_t prev_spdt_pa0 = 0b00000001;
uint8_t prev_spdt_pa1 = 0b00000100;

void TCNT0_INIT(){
	TCNT0 = 0x63;					//Set TCNT0 the value of 99(0x63) = 255 - (1 ms)*(1 MHz)/64(prescale)
	TCCR0 = (1<<CS01)|(1<<CS00);	//Set prescale 64 - these bits of TCCR0 are set - CS02 = 0  CS01 = 1  CS00 = 1
	TIMSK |= (1<<TOIE0);			//TOIE1 is set to 1 in order to enable the timer overflow interrupt
	
	DDRA = 0x04;	//Set as output PORT2 as output 
	PORTA = 0x00;	//Initialize PORTC with 0x00 WHICH mean that none of the digits has enable
	
	return;
}

int main(void){
	cli();
    TCNT0_INIT();
    sei();
	PORTA |= prev_spdt_pa0;
	PORTA |= prev_spdt_pa1;
	while(1){
    }
}

ISR(TIMER0_OVF_vect){
	if(prev_spdt_pa0 == is_bit_set(PORTA, PA0) && prev_spdt_pa1 != is_bit_set(PORTA, PA1)){
		if(is_bit_set(prev_spdt_pa1, PA1) && is_bit_clear(PORTA, PA1)){		//The previous value was 1 and now we have 0
			clear_bit(out_val, PA2);
		
		}else if(is_bit_clear(prev_spdt_pa1, PA1) && is_bit_set(PORTA, PA1)){	//The previous value was 0 and now we have 1
			set_bit(out_val, PA2);
			
		}else if(is_bit_set(prev_spdt_pa1, PA1) && is_bit_set(PORTA, PA1)){		//The previous value was 1 and now we have 1
			if(is_bit_set(prev_spdt_pa1, PA1)){
				set_bit(out_val, PA2);
			}else{
				clear_bit(out_val, PA2);
			}			
		}else{
			//Hopefully we never go there!
		}
		
		count_changes = 0;
		prev_spdt_pa1 = (PORTA & (1<<PA1));	
	}else if(prev_spdt_pa1 == is_bit_set(PORTA, PA1) && prev_spdt_pa0 != is_bit_set(PORTA, PA0)){
		if(is_bit_set(prev_spdt_pa0, PA0) && is_bit_clear(PORTA, PA0)){		//The previous value was 1 and now we have 0
			clear_bit(out_val, PA2);
			
		}else if(is_bit_clear(prev_spdt_pa0, PA0) && is_bit_set(PORTA, PA0)){	//The previous value was 0 and now we have 1
			set_bit(out_val, PA2);
			
		}else if(is_bit_set(prev_spdt_pa0, PA0) && is_bit_set(PORTA, PA0)){		//The previous value was 1 and now we have 1
			if(is_bit_set(prev_spdt_pa0, PA2)){
				set_bit(out_val, PA2);
			}else{
				clear_bit(out_val, PA2);
			}
		}else{
			//Hopefully we never go there!
		}
				
		count_changes = 0;
		prev_spdt_pa0 = (PORTA & (1<<PA0));
	}else{
		count_changes++;
	}
	
	if (count_changes == 0x0A){ //Change output after 10ms
		if(is_bit_set(out_val, PA2)){
			set_bit(PORTA, PA2);
		}else{
			clear_bit(PORTA, PA2);
		}
	}
	
	TCNT0 = 0x63;
}