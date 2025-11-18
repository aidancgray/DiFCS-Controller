#ifndef pidcontroller
#define pidcontroller
#include <parameters.h>
#include <math.h>

/*****************************************************************************/
/* UPDATE the Control Variable (CV)                                          */
/*****************************************************************************/
void pid_update (channelMap ch){
   float err = PID[ch].SP - PID[ch].PV;  // error = set value - measured
   
   PID[ch].I += err*PID[ch].kI;
   if (PID[ch].I > op_upper_bound) PID[ch].I = op_upper_bound; // anti-windup
   else if (PID[ch].I < op_lower_bound) PID[ch].I = op_lower_bound;
   
   float P = err * PID[ch].kP;  // proportional value
   
   float D = PID[ch].kD * (PID[ch].PV - PID[ch].PVold);  // derivative value
   
   PID[ch].CV = P + PID[ch].I + D;  // new control variable
   if (PID[ch].CV > op_upper_bound) PID[ch].CV = op_upper_bound; // limit positive output
   else if (PID[ch].CV < op_lower_bound) PID[ch].CV = op_lower_bound; // limit negative output
}

/*****************************************************************************/
/* PID Task - looks for change to PV and updates CV                          */
/*****************************************************************************/
void pid_task(channelMap ch){
   static int index = 0;
   if (PID[ch].SP != PID[ch].PV) pid_update(ch);
   if ((index++) >= numChannels) index = 0;
}

#endif
