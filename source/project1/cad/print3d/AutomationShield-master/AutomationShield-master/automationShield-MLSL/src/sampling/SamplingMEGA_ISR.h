/*
  ISR for handling the interrupt-driven sampling for 
  real-time control on the Arduino Mega 2560.
  
  This code is part of the AutomationShield hardware and software
  ecosystem. Visit http://www.automationshield.com for more
  details. This code is licensed under a Creative Commons

  Gergely Takacs, 2019
  Last update: 3.6.2019.
*/

#ifndef SAMPLINGMEGA_ISR_H
#define SAMPLINGMEGA_ISR_H

ISR(MEGA_ISR_VECT)
{
 if (!Sampling.fireFlag){                   // If not over the maximal resolution of the counter
  (Sampling.getInterruptCallback())();      // Start the interrupt callback
 }                                          
 else if(Sampling.fireFlag){                // else, if it is over the resolution of the counter
   Sampling.fireCount++;                    // start counting
   if (Sampling.fireCount>=Sampling.getSamplingMicroseconds()/Sampling.fireResolution){ // If done with counting
      Sampling.fireCount=0;                 // make the counter zero again
      (Sampling.getInterruptCallback())();  // and start the interrupt callback
  } 
 }
}
#endif