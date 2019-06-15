/*
 * Xmega overclock test.c
 *
 * Created: 10-6-2019 20:31:44
 * Author : Christiaan
 */ 
#define F_CPU 48000000
#include "clksys_driver.h"
#include <avr/io.h>


static void InitClocks(void)
{
	CLKSYS_XOSC_Config( OSC_FRQRANGE_12TO16_gc, 0, OSC_XOSCSEL_XTAL_16KCLK_gc );
	CLKSYS_Enable( OSC_XOSCEN_bm );
	
	do {} while ( CLKSYS_IsReady( OSC_XOSCRDY_bm ) == 0 );
	CLKSYS_PLL_Config( OSC_PLLSRC_XOSC_gc, 3);
	CLKSYS_Enable( OSC_PLLEN_bm );
	
	do {} while ( CLKSYS_IsReady( OSC_PLLRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_PLL_gc );
	CLKSYS_Disable( OSC_RC2MEN_bm );
	CLKSYS_Disable( OSC_RC32MEN_bm );
	
	}



int main(void)
{
	
	PORTD_DIRSET = PIN7_bm;
	//PORTCFG.CLKEVOUT = PORTCFG_CLKOUT_PD7_gc;
	InitClocks();
    /* Replace with your application code */
    while (1) {
		_delay_ms(1000);

		PORTD.OUTSET = PIN7_bm;
		_delay_ms(1000);
		PORTD.OUTCLR = PIN7_bm;

    }
}

