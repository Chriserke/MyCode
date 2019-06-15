#define F_CPU     2000000UL
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "serialF0.h"

void set_usartctrl(USART_t *usart, uint8_t bscale, uint16_t bsel)
{
  usart->BAUDCTRLA = (bsel & USART_BSEL_gm);
  usart->BAUDCTRLB = ((bscale << USART_BSCALE0_bp) & USART_BSCALE_gm) |
                     ((bsel >> 8) & ~USART_BSCALE_gm);
}

void init_uart_bscale_bsel(USART_t *usart, int8_t bscale, int16_t bsel)
{
  PORTD.DIRSET = PIN7_bm;          // TXD
  PORTD.DIRCLR = PIN6_bm;          // RXD (not used)

  usart->CTRLA = 0;                // no interrupts
  usart->CTRLB = USART_TXEN_bm;    // enable transmitter
  usart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc|
                     USART_PMODE_DISABLED_gc|USART_CHSIZE_8BIT_gc;
  set_usartctrl(usart, bscale, bsel);
}

int main(void)
{
	uint16_t c;
	PORTD.DIRCLR = PIN2_bm;
	PORTC.DIRSET = PIN0_bm;
	init_stream(F_CPU);
	sei();
	init_uart_bscale_bsel(&USARTD1, -7, 289);     // BAUD RATE 115200  -7 , 11

	  while (1) {
		  
		  if (PORTD.IN & PIN2_bm){									//Quit button
			  USARTD1.DATA = 'z';
			  PORTC.OUTSET = PIN0_bm;
		  }	
		  
		  else 	
				PORTC.OUTCLR = PIN0_bm;
		  
		  if ( (c = uartF0_getc()) == UART_NO_DATA ) { 			//wait for data
			  continue;
		  }
		  

		while ( ! (USARTD1.STATUS & USART_DREIF_bm) ) ;		//send data
			USARTD1.DATA = c;

	  }
  }