/*
 * lab9b.c
 *
 * Created: 12/12/2020 12:59:12 μμ
 * Author : Δημήτρης
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 10000000

#define is_bit_set(byte,bit)	(byte & (1 << bit))
#define is_bit_clear(byte,bit)	(!(byte & (1 << bit)))
#define set_bit(byte,bit)	((byte) |=  (1<<(bit)))
#define clear_bit(byte,bit)	((byte) &= ~(1<<(bit)))
#define flip_bit(byte,bit)  ((byte) ^=  (1<<(bit)))

#define out_val 0b00000100

void INIT_EXT_INTERRUPT(){
	GICR |= (1<<INT0)|(1<<INT1);	//Enable external interrupt INT0, INT1
	MCUCR = (1<<ISC11)|(1<<ISC01);	//Falling edge will generate an interrupt for both inputs PD2 and PD3
	DDRA |= (1<<PA2);
	return;
}

int main(void){
	cli();
	INIT_EXT_INTERRUPT();
	sei();
	PORTA = out_val;
    while(1){
		
    }
}

ISR(INT0_vect){
	if(is_bit_set(PORTD,PD3)){
		clear_bit(PORTA,PA2);
		
	}else if(is_bit_clear(PORTD,PD3)){
		set_bit(PORTA,PA2);
		
	}else{
		// Hopefully never goes there
	}
}

ISR(INT1_vect){
	if(is_bit_set(PORTD,PD2)){
		set_bit(PORTA,PA2);
		
	}else if(is_bit_clear(PORTD,PD2)){
		clear_bit(PORTA,PA2);
		
	}else{
		// Hopefully never goes there
	}
}