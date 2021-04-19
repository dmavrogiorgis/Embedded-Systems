;
; lab3.asm
;
; Created: 20/10/2020 8:47:30 πμ
; Author : Δημήτρης
;

.dseg
.org 0x60
	seg_dec_addr: .byte 11		//Decoded bcd to 7-seg
.org 0x80
	data_addr: .byte 8		//8 numbers to display
.org 0x90
	ok_msg_addr: .byte 4	//Here we store ok in memory

.cseg
.org 0x00			//Address of the beginning of the program
	rjmp MEM_TCNT_INIT
.org 0x12			// Address of TIMER0 OVF interrupt handler
	rjmp TIMER0_OVF
.org 0x16			// Address of USART RXC interrupt handler
	rjmp USART_RXC

.equ F_CPU = 10000000

.equ A_HEX = 0x41
.equ C_HEX = 0x43
.equ K_HEX = 0x4B
.equ N_HEX = 0x4E
.equ O_HEX = 0x4F
.equ T_HEX = 0x54
.equ X_HEX = 0x58		// Used to indicate that we have stored a number - we don't care what number
.equ LF_HEX = 0xA
.equ CR_HEX = 0xD

.def digit_cnt = r20
.def read_char = r21

MEM_TCNT_INIT:
			ldi r16, HIGH(ramend)
			out sph, r16
			ldi r16, LOW(ramend)
			out spl, r16

			ldi r16, 0x63	// Set TCNT0 the value of 99(0x63) = 255 - (1 ms)*(1 MHz)/64(prescale)
			out TCNT0, r16

			ldi r16, (1<<CS01)|(1<<CS00)	//Set prescale 64 - these bits of TCCR0 are set - CS02 = 0  CS01 = 1  CS00 = 1
			out TCCR0, r16

			ldi r16, (1<<TOIE0)	// TOIE0 is set to 1 in order to enable the timer overflow interrupt
			out TIMSK, r16	

			ldi r17, 0xFF
			ldi r18, 0x00

			out DDRA, r17		//
			out DDRC, r17		//

			out PORTA, r17		//
			out PORTC, r18		//

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

			ldi r16, 0xFF
			sts seg_dec_addr+10, r16

			ldi r16, O_HEX
			sts ok_msg_addr, r16
	
			ldi r16, K_HEX
			sts ok_msg_addr+1, r16
	
			ldi r16, CR_HEX
			sts ok_msg_addr+2, r16

			ldi r16, LF_HEX
			sts ok_msg_addr+3, r16

			ldi digit_cnt, 0x00 //Initialize digit counter to 0x00		

			ldi XL, LOW(seg_dec_addr)
			ldi XH, HIGH(seg_dec_addr)

			ldi YL, LOW(data_addr)
			ldi YH, HIGH(data_addr)

			rcall MEM_CLEAR

USART_INIT:
		cli		// Disable Global Interrupt before initialization
		
		ldi r16, (1<<RXCIE)|(1<<RXEN)|(1<<TXEN)		//Enable RXC interrupts, Receive and Transmit
		out UCSRB, r16
		
		ldi r16, (1<<URSEL)		// We make it "1" in order to write to UCSRC 
		out UCSRC, r16
		ldi r16, (1<<UCSZ0)|(1<<UCSZ1)	//8-bit size for everything we transmit and receive
		out UCSRC, r16

		//The baud rate parameter is assumed to be stored in the r17:r16 registers
		ldi r16, 0x41	// 9600 baud rate means we have to set 65 to UBRRL
		ldi r17, 0x00
		out UBRRH, r17
		out UBRRL, r16

		sei		// Enable Global Interrupt before initialization

main:
	nop			//We are doing no op while we wait for the interrupt
	rjmp main

MEM_CLEAR:
		ldi ZL, LOW(data_addr)
		ldi ZH, HIGH(data_addr)
		ldi r16, 0x08
		ldi r17, 0x0A

		while_clear: 
				st Z+, r17
				dec r16
				brne while_clear
		ret 

MEM_SHIFT:
		ldi ZL, LOW(data_addr)
		ldi ZH, HIGH(data_addr)
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
	  ldi ZL, LOW(ok_msg_addr)
	  ldi ZH, HIGH(ok_msg_addr)
	  ldi r17, 0x04

	  USART_T:
		  sbis UCSRA, UDRE	//UDRE is 1 when the UDR is empty.  
		  rjmp USART_T		//So we wait untill we send all the 4 bytes

	  ld r18, Z+
	  //out UDR, r18
	  out TCNT2, r18
	  dec r17
	  brne USART_T

	  ldi read_char, 0x00
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


TIMER0_OVF: 
		cpi digit_cnt, 0x08			//Check if we have dispaly all the digits
		brne done
		ldi digit_cnt, 0x00			//Reset digit_cnt (r20) with 0 to start counting the digits again
		ldi YL, LOW(data_addr)		//Reset YL register to the first digit data address
		ldi YH, HIGH(data_addr)		//Reset YL register to the first digit data address
   done:
		ldi r17, 0xFF
		out PORTA, r17		

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

USART_RXC:
		rcall GETC

		cpi read_char, 0x00		// If read_char register has 0x00 and the value we receive is A, we store A
		brne done_1				
		cpi r18, A_HEX
		brne done_1
		mov read_char, r18	
		reti
  done_1:
		cpi read_char, 0x00		// If read_char register has 0x00 and the value we receive is C or N, we store them and clear memory
		brne done_2				
		cpi r18, C_HEX
		breq if_1				
		cpi r18, N_HEX
		brne done_2
   if_1:
		mov read_char, r18		
		rcall MEM_CLEAR			
		reti
  done_2:
		cpi read_char, A_HEX	// If read_char register has A and the value we receive is T, we replace A with T
		brne done_3
		cpi r18, T_HEX
		brne done_3
		mov read_char, r18	
		reti
  done_3:
		cpi r18, CR_HEX			//If read_char register has CR and the value we receive is T or C or X (X means number), 
		brne done_4				//we replace them with CR
		cpi read_char, T_HEX	
		breq if_2
		cpi read_char, C_HEX	
		breq if_2
		cpi read_char, X_HEX	
		brne done_5	
   if_2:
		mov read_char, r18
		reti
  done_4:
		cpi r18, 0x30		//If we receive a number 0-9 and the the read_char is N or an other number, we replace them with X
		brlt done_5			//and then we shift data in memory, store the new number and return
		cpi r18, 0x3A		
		brge done_5
		cpi read_char, N_HEX
		breq if_3			
		cpi read_char, X_HEX
		brne done_5				
    if_3:
		rcall MEM_SHIFT
		subi r18, 0x30			// The mask we need to apply in order to isolate the bcd form of the number
		sts data_addr, r18		
		ldi read_char, X_HEX	
		reti
  done_5:
		cpi read_char, CR_HEX	//If  read_char register has CR and the value we receive is LF, we send OK<CR><LF>
		brne done_6
		cpi r18, LF_HEX
		brne done_6
		rcall SEND_OK
		reti
  done_6:
		ldi read_char, 0x00		//We clear read_char register if we receive an unknown char without the correct order
		reti