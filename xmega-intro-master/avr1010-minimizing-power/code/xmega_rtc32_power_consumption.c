/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief  XMEGA power consumption validation main program file for 32-bit RTC
 *
 *      This file contains a simple program for measuring the current
 *      consumption of XMEGA devices in different sleep modes.\n
 *      The device cycles through ACTIVE, IDLE, POWER-SAVE and POWER-DOWN
 *      modes, spending 8 seconds in each mode.\n
 *      Note that the device will stay in POWER-DOWN.
 *
 *      The interval timing is done with the 32-bit RTC, clocked by the external 
 *	32 kHz crystal oscillator.
 *
 *      \note The device is initialized with the LOWPOWER_Init(), which enables
 *      pull-up on all I/O pins and disables the JTAG-interface (if configured
 *      in lowpower.h). This results in the lowest current consumption if
 *      nothing is connected to the I/O pins.
 *
 * \par Application note:
 *      AVR1010: Minimizing the power consumption of XMEGA devices
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Revision: 2940 $
 * $Date: 2009-11-02 09:55:56 +0100 (ma, 02 nov 2009) $  \n
 *
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/


#include "avr_compiler.h"
#include "ccpwrite.h"
#include "sleepmgr.h"
#include "lowpower.h"
#include "rtc32_driver.h"


//! Configure RTC wakeup period in seconds. (Approximate if ULP is used..)
#define RTC_PERIOD	8

//! Define the CPU frequency, for use with delay_us(). (2 MHz is default clock.)
#ifndef F_CPU
#define F_CPU 2000000
#endif // F_CPU


//! Global variable to keep track of which state the program is in.
volatile uint8_t gState = 0;


/*! \brief The main RTC example.
 *
 * This function initializes the XMEGA to the least power consuming state,
 * before initializing the sleep manager and RTC.\n
 * The function waits for gState to change (due to RTC compare ISR), after
 * which an infinite loop is entered in which the device is put to sleep.
 */
int main(void)
{
    // Initialize device to the least power consuming state.
    LOWPOWER_Init();
    
    // Enable Flash and EEPROM power reduction modes.
    CCPWrite(&NVM.CTRLB, NVM_EPRM_bm | NVM_FPRM_bm);
    
    // Initialize the sleep manager.
    SLEEPMGR_Init();

    // Clear bit for RTC in PRR (it is set by LOWPOWER_Init()).
    PR.PRGEN &= ~PR_RTC_bm;

    // Reset the battery backup module.
    RTC32_Reset();

    // Configure and enable TOSC, then set up and enable the RTC32 module.
    RTC32_ToscEnable( false );
    RTC32_Initialize( RTC_PERIOD, 0, RTC_PERIOD-1 );

    // Enable RTC compare interrupts.
    RTC32_SetCompareIntLevel( RTC32_COMPINTLVL_LO_gc );
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei();

    // The device should first spend time in ACTIVE, so wait for
    // the RTC compare ISR to change the state.
    do { } while(gState == 0);
    
    // Disable Flash power reduction mode due to errata.
    // (The device will spend most of its time in sleep from now on, so we
    // won't bother clearing/setting FPRM before and after sleep.)
    CCPWrite(&NVM.CTRLB, NVM_EPRM_bm);
    
    // Go to sleep. The RTC compare ISR configures the sleep modes.
    do {
        SLEEPMGR_Sleep();
    } while (1);
}


/*! RTC compare ISR
 *
 * This ISR simply updates gState so that the device cycles through the
 * different sleep modes.
 */
ISR(RTC32_COMP_vect)
{
    switch(gState) {
        // The device starts out in active mode. Go to Idle.
        case 0:
            SLEEPMGR_Lock( SLEEPMGR_IDLE );
            ++gState;
            break;
                
        // Power-save follows after Idle.
        case 1:
            SLEEPMGR_Unlock( SLEEPMGR_IDLE );
            SLEEPMGR_Lock( SLEEPMGR_SAVE );
            ++gState;
            break;
                
        // Power-down follows after Power-save.
        case 2:
            SLEEPMGR_Unlock( SLEEPMGR_SAVE );
            SLEEPMGR_Lock( SLEEPMGR_DOWN );
            ++gState;

            // Disable interrupt to remain in sleep. (RTC32 will keep running.)
            RTC32_SetCompareIntLevel( RTC32_COMPINTLVL_OFF_gc );
            break;
        
        // Shouldn't end up here.. Go to Idle.
        default:
            SLEEPMGR_Init();
            SLEEPMGR_Lock( SLEEPMGR_IDLE );
            gState = 1;
    }
}
