;
; lab1a.asm
;
; Created: 3/10/2020 9:10:32 μμ
; Author : Dimitris Mavrogiorgis
; AM     : 2016030016
;
.equ F_CPU=10000000    //Set clock freq to 10 MHz 

ldi r16, high(ramend)	// Set stack pointer to the top of the ram
out sph, r16
ldi r16, low(ramend)
out spl, r16

sbi DDRB, 0 // Set PB0 as an output

start:
    cbi PORTB, 0		// Start with 0 output in pin PB0 
	rcall delay			//rcall takes 3 cycles 
	sbi PORTB, 0		//Set with 1 output in pin PB0
	rcall delay 
	rjmp start

delay: ldi r18, 0x0A	// Set r18 the value 10(0x0A)
here:  ldi r17, 0xFA	//Set r17 the value 250(0xFA)
again: nop				//1 cycle
	   dec r17			//1 cycle
	   brne again		//2 cycles
	   dec r18			//1 cycle
	   brne here		//2 cycles
	   ret				//4 cycles
