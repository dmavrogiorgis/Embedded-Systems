;
; lab1b.asm
;
; Created: 4/10/2020 12:47:51 μμ
; Author : Dimitris Mavrogiorgis
; AM     : 2016030016
;
.equ F_CPU=10000000 

.dseg
.org 0x60
	seg_dec_addr: .byte 10
.org 0x76
	data_addr: .byte 8

.def digit_cnt = r20

.cseg
.org 0x00			//Address of the beginning of the program
	rjmp start
.org 0x12			// Address ofthe interrupt handler
	rjmp handler

start:
	ldi r16, HIGH(ramend)
	out sph, r16
	ldi r16, LOW(ramend)
	out spl, r16

    ldi r16, 0x63	// Set TCNT0 the value of 99(0x63) = 255 - (1 ms)*(1 MHz)/64(prescale)
	out TCNT0, r16

	ldi r16, 0x03	//Set prescale 64 - these bits of TCCR0 are set - CS02 = 0  CS01 = 1  CS00 = 1
	out TCCR0, r16

	ldi r16, 0x01	// TOIE0 is set to 1 in order to enable the timer overflow interrupt
	out TIMSK, r16	

	ldi r17, 0xFF
	ldi r21, 0x00

	out DDRA, r17		//
	out DDRC, r17		//

	out PORTA, r17		//
	out PORTC, r21		//

	sei		// Enable global interrupt

	ldi r16, 0xC0				//bcd to 7-segment decoding and storing in sram
	sts seg_dec_addr, r16

	ldi r16, 0xF9
	sts seg_dec_addr+1, r16

	ldi r16, 0xA4
	sts seg_dec_addr+2, r16

	ldi r16, 0xB0
	sts seg_dec_addr+3, r16

	ldi r16, 0x99
	sts seg_dec_addr+4, r16

	ldi r16, 0x92
	sts seg_dec_addr+5, r16

	ldi r16, 0x82
	sts seg_dec_addr+6, r16

	ldi r16, 0xF8
	sts seg_dec_addr+7, r16
	
	ldi r16, 0x80
	sts seg_dec_addr+8, r16
	
	ldi r16, 0x90
	sts seg_dec_addr+9, r16
	
	ldi r16, 0x01				//Start storing the 8 numbers to display from 1 to 8
	sts data_addr, r16

	inc r16
	sts data_addr+1, r16
	
	inc r16
	sts data_addr+2, r16

	inc r16
	sts data_addr+3, r16

	inc r16
	sts data_addr+4, r16

	inc r16
	sts data_addr+5, r16

	inc r16
	sts data_addr+6, r16

	inc r16
	sts data_addr+7, r16

	ldi digit_cnt, 0x00 //Initialize digit counter to 0x00

	ldi XL, LOW(seg_dec_addr)
	ldi XH, HIGH(seg_dec_addr)

	ldi YL, LOW(data_addr)
	ldi YH, HIGH(data_addr)

	rjmp main


main:
	nop			//We are doing no op while we wait for the interrupt
	rjmp main

handler: 
		cpi digit_cnt, 0x08			//Check if we have dispaly all the digits
		brne done
		ldi digit_cnt, 0x00			//Reset digit_cnt (r20) with 0 to start counting the digits again
		ldi YL, LOW(data_addr)		//Reset YL register to the first digit data address
		ldi YH, HIGH(data_addr)		//Reset YL register to the first digit data address
   done:
		out PORTA, r17		//r17 has the value 0xFF from initialization of DDRA-C and PORTA-C

		ld r19, Y+		//Read the bcd number 

		add XL, r19		//Calculate the address in which the decoded number exists
		ld r18, X
		sub XL, r19		//

		cpi digit_cnt, 0x00		//Check if we have to dispaly the 1st digit
		brne else
		in r16, PORTC
		sec					//Set carry for ROL instr
		rol r16	
		clc					//Clear carry
		out PORTC, r16						
		jmp done1
   else:					//We already have one "1" and we need to just shift left
		in r16, PORTC		
		rol r16
		out PORTC, r16		
   done1:
   		out PORTA, r18

		ldi r16, 0x63		//Set once again the value of TCNT0 in order to roll again after the handling
		out TCNT0, r16

		inc digit_cnt
		
		reti