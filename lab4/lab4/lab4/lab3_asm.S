/*
 * lab4.c
 * Author : Dimitris Mavrogiorgis
 * AM : 2016030016
 */ 
#include <avr/io.h> #undef __SFR_OFFSET#define __SFR_OFFSET 0

#define A_HEX 0x41
#define C_HEX 0x43
#define K_HEX 0x4B
#define N_HEX 0x4E
#define O_HEX 0x4F
#define T_HEX 0x54
#define X_HEX 0x58		// Used to indicate that we have stored a number - we don't care what number
#define LF_HEX 0x0A
#define CR_HEX 0x0D

.global MEM_INIT
.global MEM_CLEAR
.global TIMER0_OVF_vect.global USART_RXC_vect

.section .text  

MEM_INIT:
		ldi XL, lo8(seg_dec_addr)
		ldi XH, hi8(seg_dec_addr)

		ldi YL, lo8(data_addr)
		ldi YH, hi8(data_addr)
		ret

MEM_CLEAR:
		ldi ZL, lo8(data_addr)
		ldi ZH, hi8(data_addr)
		ldi r16, 0x08
		ldi r17, 0x0A

		while_clear: 
				st Z+, r17
				dec r16
				brne while_clear
		ret 

MEM_SHIFT:
		ldi ZL, lo8(data_addr)
		ldi ZH, hi8(data_addr)
		ldi r16, 0x06
		add ZL, r16

		ldi r16, 0x07

		while_shift: 
				ld r17, Z+
				st Z, r17
				subi ZL, 0x02
				dec r16
				brne while_shift
		ret

SEND_OK:
	  ldi ZL, lo8(ok_msg_addr)
	  ldi ZH, hi8(ok_msg_addr)
	  ldi r17, 0x04

	  USART_T:
		  sbis UCSRA, UDRE	//UDRE is 1 when the UDR is empty.  
		  rjmp USART_T		//So we wait untill we send all the 4 bytes

	  ld r18, Z+
	  //out UDR, r18
	  out TCNT2, r18
	  dec r17
	  brne USART_T

	  ldi r21, 0x00
	  sts read_char, r21
	  ret

GETC: 
	in r17, UCSRA
	sbrs r17, RXC
	rjmp GETC

	in r18, UDR
	in r18, UDR ; added
	mov r18, r15 ; added
	ret

 PUTC: 
	in r17, UCSRA
	sbrs r17, UDRE
	rjmp PUTC

	; out UDR, r16
	out TCNT2, r16 ; replaced
	ret

TIMER0_OVF_vect: 
		lds r20, digit_cnt

		cpi r20, 0x08			//Check if we have dispaly all the digits
		brne done
		ldi r20, 0x00			//Reset digit_cnt (r20) with 0 to start counting the digits again
		ldi YL, lo8(data_addr)		//Reset YL register to the first digit data address
		ldi YH, hi8(data_addr)		//Reset YL register to the first digit data address
   done:
		ldi r17, 0xFF
		out PORTA, r17		

		ld r19, Y+		//Read the bcd number 

		add XL, r19		//Calculate the address in which the decoded number exists
		ld r18, X
		sub XL, r19		//

		cpi r20, 0x00		//Check if we have to dispaly the 1st digit
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

		inc r20
		sts digit_cnt, r20
		reti

USART_RXC_vect:
		rcall GETC

		lds r21, read_char

		cpi r21, 0x00		// If read_char register has 0x00 and the value we receive is A, we store A
		brne done_1				
		cpi r18, A_HEX
		brne done_1
		mov r21, r18	
		sts read_char, r21
		reti
  done_1:
		cpi r21, 0x00		// If read_char register has 0x00 and the value we receive is C or N, we store them and clear memory
		brne done_2				
		cpi r18, C_HEX
		breq if_1				
		cpi r18, N_HEX
		brne done_2
   if_1:
		mov r21, r18		
		rcall MEM_CLEAR	
		sts read_char, r21		
		reti
  done_2:
		cpi r21, A_HEX	// If read_char register has A and the value we receive is T, we replace A with T
		brne done_3
		cpi r18, T_HEX
		brne done_3
		mov r21, r18
		sts read_char, r21	
		reti
  done_3:
		cpi r18, CR_HEX			//If read_char register has CR and the value we receive is T or C or X (X means number), 
		brne done_4				//we replace them with CR
		cpi r21, T_HEX	
		breq if_2
		cpi r21, C_HEX	
		breq if_2
		cpi r21, X_HEX	
		brne done_5	
   if_2:
		mov r21, r18
		sts read_char, r21
		reti
  done_4:
		cpi r18, 0x30		//If we receive a number 0-9 and the the read_char is N or an other number, we replace them with X
		brlt done_5			//and then we shift data in memory, store the new number and return
		cpi r18, 0x3A		
		brge done_5
		cpi r21, N_HEX
		breq if_3			
		cpi r21, X_HEX
		brne done_5				
    if_3:
		rcall MEM_SHIFT
		subi r18, 0x30			// The mask we need to apply in order to isolate the bcd form of the number
		sts data_addr, r18		
		ldi r21, X_HEX
		sts read_char, r21	
		reti
  done_5:
		cpi r21, CR_HEX	//If  read_char register has CR and the value we receive is LF, we send OK<CR><LF>
		brne done_6
		cpi r18, LF_HEX
		brne done_6
		rcall SEND_OK
		reti
  done_6:
		ldi r21, 0x00		//We clear read_char register if we receive an unknown char without the correct order
		sts read_char, r21
		reti