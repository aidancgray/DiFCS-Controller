#ifndef timeout
#define timeout
#include <parameters.h>
/*****************************************************************************/
/*****************************************************************************/           
/* Global timers used by all routnes                                         */
/* timeoutReg0-2 are used for timeouts. Other routines monitor the value of  */
/* intTimeoutReg. On boot, they will be loaded from EEProm. Any              */
/* configuation changes will be changed in EEprom, hence will be restored on */
/* subsequent boot.                                                          */
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Register definitions                                                      */
/*****************************************************************************/
//!unsigned int32 timeoutReg0 = 0; //All timers are defined in milliseconds, 
//!unsigned int32 timeoutReg1 = 0; //with a resolution of 10ms
//!unsigned int32 timeoutReg2 = 0;
//!unsigned int16 intTimeoutReg = 0; // interrupting use for more precise timing

/*****************************************************************************/
/* TIMEOUT_INIT initializes the timer                                        */
/*****************************************************************************/
void timeout_init()
{        
   disable_interrupts(INT_TIMER1);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   timeoutReg0 = 0;  // clear all the timers
   timeoutReg1 = 0;
   timeoutReg2 = 0;
   intTimeoutReg = 0;
   enable_interrupts(INT_TIMER1);
}

/*****************************************************************************/
/* TIMEOUT INTERRUPT decrements the timeout registers every second           */
/*****************************************************************************/
#INT_TIMER1
void TIMER1_isr(void) 
{
   static timeCount = 0;
   set_timer1(0x0BDB); // load timer register to allow 10 millisecond interupts
  
   if (timeoutReg0 > 10) timeoutReg0 -= 10; else timeoutReg0 = 0;
   if (timeoutReg1 > 10) timeoutReg1 -= 10; else timeoutReg1 = 0;
   if (timeoutReg2 > 10) timeoutReg2 -= 10; else timeoutReg2 = 0;
   if (intTimeoutReg != 0) // INTERRUPTING TIMER
   {
      timeCount+=10;
      if (timeCount >= intTimeoutReg)
      {
         sensor_monitor_interrupt_task();
         timeCount = 0;
      }
   }
}

#endif
