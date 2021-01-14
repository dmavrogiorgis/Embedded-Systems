;
; lab1b.asm
;
; Created: 4/10/2020 12:47:51 μμ
; Author : Dimitris Mavrogiorgis
; AM     : 2016030016
;
.equ F_CPU=10000000 

.org 0x00			//Address of the beginning of the program
	rjmp start
.org 0x12			// Address ofthe interrupt handler
	rjmp handler

start:
	ldi r16, high(ramend)
	out sph, r16
	ldi r16, low(ramend)
	out spl, r16

    ldi r16, 0x63	// Set TCNT0 the value of 99(0x63) = 255 - (1 ms)*(1 MHz)/64(prescale)
	out TCNT0, r16

	ldi r16, 0x03	//Set prescale 64 - these bits of TCCR0 are set - CS02 = 0  CS01 = 1  CS00 = 1
	out TCCR0, r16

	ldi r16, 0x01	// TOIE0 is set to 1 in order to enable the timer overflow interrupt
	out TIMSK, r16	

	sbi DDRB, 0		// Set PB0 as an output
	cbi PORTB, 0	//Initiate the output as 0

	sei		// Enable global interrupt

	rjmp main

main:
	nop			//We are doing no op while we wait for the interrupt
	rjmp main

handler: 
		in r16, PORTB
		com r16				//Set the opposite value in r16 - eg from 0 -> 1 or 1 -> 0
		andi r16, 0x01		//Set only bit 0 of r16 the opposite value and dont change the other
		out PORTB, r16

		ldi r16, 0x63		//Set once again the value of TCNT0 in order to roll again after the handling
		out TCNT0, r16
		reti