//  https://gyazo.com/56b56d40bad59aa4079ac7788c02ddbc propagation delay
//  https://github.com/Synapseware/xmega-intro  Avr examples PLL clock 
//  https://mil.ufl.edu/3744/classes/xmega_clk_system_avr1003.pdf avr document for system clock and PLL
//	https://www.microchip.com/wwwproducts/en/ATxmega256A3U avr notes and docouments


// TO DO test overflow interupt met adc conversione done interupt in een while ipv interupt op channel 0 ADC
// IF TCO interupt & ADC interupt DO FILTER

#define F_CPU 48000000
#include "clksys_driver.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//ADC_voltage:
#define MAX_VALUE_ADC   2047                               // only 11 bits are used
#define VCC         3.30
#define VREF        (((double) VCC) / 1.6)                 // maximal value is 2.06125 V
//DAC
#define MAX_VALUE_DAC 4095
#define CAL_DAC 1.000  // Calibration value DAC


int16_t testvariable = 0;
volatile uint16_t  voltage;
volatile uint16_t BinaryValue;// contains value to write to DAC

int16_t state_x1, state_x2, state_y1, state_y2;
int16_t state_x2_1, state_x2_2, state_y2_1, state_y2_2;

int32_t y2;
int32_t y;
//1st 2nd order
int32_t a11 = -28286;
int32_t a12 = 12253;
int32_t b10 = 88;
int32_t b11 = 176;
int32_t b12 = 88;
//2nd 2nd order
int32_t a21 = -30541;
int32_t a22 = 14537;
int32_t b20 = 95;
int32_t b21 = 190;
int32_t b22 = 95;
//float a11 = -1.725933395036940880;
//float a12 = 0.747447371907791203 ;
//float b10 =  0.005378494217712604;
//float b11 = 0.010756988435425208;
//float b12 = 0.005378494217712604;
//
//float a21 = -1.863800492075235350;
//float a22 =0.887032999652694887;
//float b20 = 0.005808126894364891;
//float b21 = 0.011616253788729781;
//float b22 = 0.005808126894364891;

volatile int16_t res;

void init_timer(void){
	TCC0.PER = 2400;    // 48MHz/2400 = 20KHz
	TCC0.INTCTRLA = TC_ERRINTLVL_OFF_gc | TC_OVFINTLVL_OFF_gc; /* All timer interrupts off */
	TCC0.INTCTRLB = TC_CCAINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCDINTLVL_OFF_gc; /* Disable Compare/Capture interrupts */
	TCC0.CNT = 0;
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc; /* Start the timer with a clock divider of 1 */
	
	EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc; // Connect TCC0 overflow to event channel 0, allowing us to trigger on this interupt  http://asf.atmel.com/docs/latest/xmegaa/html/adc_use_case_2.html
  //TCE0.PER      = 2400;     
  //TCE0.CTRLA    = TC_CLKSEL_DIV1_gc;          // Prescaling 8
  //TCE0.CTRLB    = TC_WGMODE_NORMAL_gc;        // Normal mode
  //TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;        // Interrupt overflow off
}

void init_dac(void)
{
	DACB.CTRLC = DAC_REFSEL_AVCC_gc;					// Select reference
	DACB.CTRLB = DAC_CHSEL_SINGLE_gc;					// Enable single channel.
	DACB.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;			// Enable dac and CH0
}


void init_adc(void)
{
	PORTA.DIRCLR     = PIN2_bm|PIN3_bm;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc |										// PA2 (PIN A2) to + channel 0
	ADC_CH_MUXNEG_GND_MODE3_gc;														// GND to - channel 0
	ADCA.CH0.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_DIFF_gc;					/* Diff in, not using the gainblock */
	ADCA.REFCTRL     = ADC_REFSEL_INTVCC_gc;
	ADCA.CTRLB = ADC_CURRLIMIT_NO_gc | ADC_RESOLUTION_12BIT_gc ;
	ADCA.CTRLB = ADC_CONMODE_bm;	/* Enable signed mode, 12-bit conversion */
	ADCA.EVCTRL = ADC_SWEEP_0_gc | ADC_EVSEL_0123_gc | ADC_EVACT_SYNCSWEEP_gc;		/* Sweep channel 0, trigger using event channel 0, sync sweep on event */

	ADCA.PRESCALER = ADC_PRESCALER_DIV32_gc;										// 48MHz/32 = 1.5M
	ADCA.CTRLA       = ADC_ENABLE_bm;
	PMIC.CTRL |= PMIC_LOLVLEN_bm;													// low level interupt for adc conversion
	ADCA.CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc;			/* Trigger a low-level interrupt on completion of the CH0 conversion */

}

static void InitClocks(void){
	
	/*  https://github.com/Synapseware/xmega-intro/blob/master/avr1003-clock-system/code/clksys_example.c    */
	CLKSYS_XOSC_Config( OSC_FRQRANGE_12TO16_gc, 0, OSC_XOSCSEL_XTAL_16KCLK_gc );  // Set frequency ranger, Low power off, Slow startup
	CLKSYS_Enable( OSC_XOSCEN_bm );												  // Enable Clock
	
	do {} while ( CLKSYS_IsReady( OSC_XOSCRDY_bm ) == 0 );						  // When ready set PLL
	CLKSYS_PLL_Config( OSC_PLLSRC_XOSC_gc, 3);									  // set clocksource and multiplication factor,
	CLKSYS_Enable( OSC_PLLEN_bm );												  // enable the PLL
	
	do {} while ( CLKSYS_IsReady( OSC_PLLRDY_bm ) == 0 );						  // when ready select PLL clock and disable the others.
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_PLL_gc );
	CLKSYS_Disable( OSC_RC2MEN_bm );
	CLKSYS_Disable( OSC_RC32MEN_bm );
	
	} 


ISR(ADCA_CH0_vect)                                            // ADC Interupt for 20k samples, triggers on each sample on adc conversion complete.
{
		
	 DACB.CH0DATA = BinaryValue;												//write filterdata to DAC (PIN A10)
//	 DACB.CH1DATA = (~BinaryValue) & 0xFFF;										//inverse filter value to allow differential measurement

	 res = ADCA.CH0.RES;


	 voltage = ((double)res * 1000 * VREF) / (MAX_VALUE_ADC + 1);
	 //  printf("%u\n",iVoltage);

	 y2 = y2 + ((b10 * voltage) + (b11 * state_x1) + (b12*state_x2) - (a11*state_y1) - (a12*state_y2));
	 //printf("%d\n", y2);
//
	    //if (y2 > 0x1FFFFFFF)
	    //{
		    //y2 = 0x1FFFFFFF;     /* clip value */
	    //}
	    //if (y2 < -0x20000000)
	    //{
		    //y2 = -0x20000000;    /* clip value */
	    //}

	 y2 = y2 >> 14;
	 state_x2 = state_x1;             /* bump the states over */
	 state_x1 = voltage;
	 state_y2 = state_y1;
	 state_y1 = y2;

	 //2nd 2nd order
	 y = y + (b20*y2) + (b21*state_x2_1) + (b22*state_x2_2) - (a21*state_y2_1) - (a22*state_y2_2);
	 //printf("%d\n", y);

		    //if (y > 0x1FFFFFFF)
		    //{
			    //y = 0x1FFFFFFF;     /* clip value */
		    //}
		    //if (y < -0x20000000)
		    //{
			    //y = -0x20000000;    /* clip value */
		    //}
			
			//x_in is output of previous part.
	 state_x2_2 = state_x2_1;         /* bump the states over */
	 state_x2_1 = y2;
	 state_y2_2 = state_y2_1;
	 state_y2_1 = y >> 14;

	
	 double dY = y >> 14;
	 BinaryValue =  dY*((MAX_VALUE_DAC)/(VCC))*0.001*CAL_DAC ;				// Bitvalue


//"fraction saving", but could be called "first-order noise shaping with round-down quantizer": whatever bits you lose in rounding down, add those zero-extended bits into the accumulator in the following sample. 
//this way your rounding error at DC is zero. infinite S/N at DC, lowered S/N for low frequencies and slightly higher S/N for frequencies above Nyquist/2 . 
// Gives binaryvalues  up to 2550 instead of 2400 without this. Just adds the fractions that are lost when rounding down to the next sample.
	 y2 = y2 & 0x00003FFF;
	 y = y & 0x00003FFF;
	
	
 }

int main(void)
{
	 
//	PORTD.DIRSET = PIN7_bm;
	InitClocks();
	init_timer();		// init timer
	init_dac();				// init DAC
	init_adc();				// init ADC
//	PORTCFG.CLKEVOUT = PORTCFG_CLKOUT_PD7_gc; // output CLK value on pin D7




	sei();					//Enable interrupts
	while (1){

				
	}
	
}