#ifndef pidcontroller
#define pidcontroller
#include <parameters.h>
#include <math.h>

/*****************************************************************************/
/* UPDATE the Control Variable (CV)                                          */
/*****************************************************************************/
void pid_update (channelMap ch){
   float err = PID[ch].SP - PID[ch].PV;   // error = set value - measured
   PID[ch].I += (err * PID[ch].kI);       // integral value 
   float P = err * PID[ch].kP;            // proportional value
   float D = PID[ch].kD * (PID[ch].PV - PID[ch].PVold); // derivative value
   PID[ch].CV = P + PID[ch].I + D;        // new control variable
   if (PID[ch].CV > 100) 
   {
      PID[ch].CV = 100;// limit output to 100%
   }
}

/*****************************************************************************/
/* PID Task - looks for change to PV and updates CV                          */
/*****************************************************************************/
void pid_task(channelMap ch){
   static int index = 0;
   if (adcVals[ch].pReal != PID[ch].PV) pid_update(ch);
   if ((index++) >= numChannels) index = 0;
}

#endif
