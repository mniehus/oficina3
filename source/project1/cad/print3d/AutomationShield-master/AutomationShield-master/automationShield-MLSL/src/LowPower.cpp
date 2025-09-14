/*
 * API implementing low-power mode (LPM) functionality and sleep modes.
 * 
 * This file implements low-power mode (LPM) functionality and sleep
 * modes and has been only tested on the ATmega 328p MCU, e.g. works
 * only on the Arduino UNO, may be possibly suitable for other AVR
 * chips. The public methods a) cycle through GPIO to reduce active
 * current consumption b) power down the MCU with minimal sleeping 
 * current, yet enabling wake-up from  interrupts D2/D3 and c) set
 * the watchdog timer to wake the system.
 * 
 * This code is part of the AutomationShield hardware and software
 * ecosystem. Visit http://www.automationshield.com for more
 * details. This code is licensed under a Creative Commons
 * Attribution-NonCommercial 4.0 International License.
 *
 * Created by Gergely Takács
 * Last update: 12.2.2020.
 *
 * Based on the tutorials of 
 * Kevin Darrah https://www.youtube.com/watch?v=urLSDi7SD8M (power down)
 * Julian Ilett https://www.youtube.com/watch?v=iMC6eG24S9g (slowdown)
*/

#include "LowPower.h"

#ifdef ARDUINO_AVR_UNO

/* Set all GPIO pins to outputs to reduce current consumption 
 * Saves about 3 mA current. There are 19 pins on the UNO.
 */
void LowPowerClass::setPinsToOutputs(void){ 
  for (int i=1; i<20; i++){                                 // 19 In the UNO incl. 13 digital + 6 analog
    pinMode(i,OUTPUT);
  }
}

void LowPowerClass::begin(void){
  setPinsToOutputs();
}

/* Power down the MCU
 * SMCR = Sleep Mode Control Register
 * The appropriate power down mode is set by the SMx combination,
 * then the sleep is enabled by the sleep enable bit.
 * This command just sets up and enables the power down mode,
 * execution must be realized by an inline assembler command,
 * see e.g. powerDownExecute().
 */
void LowPowerClass::powerDownInitialize(void){
  SMCR |= (1<<SM1);                                       // Select powerDown mode by setting appropriate bit
  SMCR |= (1<<SE);                                        // Set enable sleep mode bit 
}

/*
 * Executes a power down command using inline 
 * assembler. Power down mode must be configured
 * and enabled before running this function. See
 * e.g. powerDownInitialize for more details.
 */
void LowPowerClass::powerDownExecute(void){
  __asm__ __volatile__("sleep");                          // Inline assembler to execute the sleep command  
}

void LowPowerClass::powerDown(void){
 LowPower.disableADC();                                   // Disables ADC
 LowPower.powerDownInitialize();                          // Initializes a power down sequence
 LowPower.disableBOD();                                   // Disables BOD
 LowPower.powerDownExecute();                             // Executes power down command
}


/* Turn off ADC completely
 * Done in the ADC status and control register A (ADCSRA) by
 * clearing the ADEN bit.
 */
void LowPowerClass::disableADC(void){
  ADCSRA &= ~(1<<ADEN);                                     // Clear the ADEN bit in the ADC status and control register A
}


/* Set watchdog timer
 * WDTCSR = WatchDog Timer Control Register. Operations are strictly timed, see datasheet.
 * Watchdog timer is actually pretty shitty with small timeouts. 16 ms is the smallest and can't do matches.
 */ 
void LowPowerClass::setWatchDog(){
 WDTCSR  = (1 << WDCE) | (1 << WDE);                        // Set watchdog change enable (WDCE) bit, sets watchdog enable bit (WDE) in a timed sequence, meanwhile clears the rest
 WDTCSR  = (1 << WDP0) | (1 << WDP3);                       // Set prescalers, meanwhile clear WDE and WDCE bits
 WDTCSR |= (1 << WDIE);                                     // Watchdog interrupt enable
}

/*
 * Disable brown out detection (BOD) during sleep
 * This can be done in the MCU control register (MCUCR)
 * It is a strictly timed sequence, bits must be written and cleared at the same time.
 * This command must be executed immediately before the "sleep" assembler command.
 */ 
void LowPowerClass::disableBOD(void){
 MCUCR |= (1 << BODS)|(1 << BODSE);                         // Set brownout detection sleep (BODS) bit and brownout detection sleeep enable (BODSE) at the same time
 MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);             // Clear BODSE and set BODS within the same time.
                                                            // assembly sleep command must immediately follow, otherwise it is not effective
}

/* Slows down crystal clock by a prescaler
 * Clock prescaler CLCKPR needs a timed sequence
 * to prevent unintentional writing of the bits.
 */
void LowPowerClass::slowDown(byte scaler){
   CLKPR = (1 << CLKPCE);   // Set the clock prescaler change enable (CLKPCE). This is a timed sequence.
if (scaler == 1){
  // No change
} 
else if (scaler == 2){
  CLKPR = (1 << CLKPS0);                                      // DIV = /2, about 10 mA 
}
else if (scaler == 4){
  CLKPR = (1 << CLKPS1);                                      // DIV = /4, about 8 mA 
}
else if (scaler == 8){
  CLKPR = (1 << CLKPS1) | (1 << CLKPS0);                      // DIV = /8
}
else if (scaler == 16){
  CLKPR = (1 << CLKPS2);                                      // DIV = /16, about 6 mA 
}
else if (scaler == 32){
  CLKPR = (1 << CLKPS2) | (1 << CLKPS0);                      // DIV = /32
}
else if (scaler == 64){
  CLKPR = (1 << CLKPS2) | (1 << CLKPS1);                      // DIV = /64
}
else if (scaler == 128){
  CLKPR = (1 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS1);      // DIV = /128
}
else if (scaler == 256){
  CLKPR = (1 << CLKPS3);                                      // DIV = /256, does not seem to work
}
else{
  AutomationShield.error("Prescaler does not exist for this architecture."); // Error message to serial line.
}
};

#endif

LowPowerClass LowPower;  // Construct instance (define)
