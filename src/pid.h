#ifndef pidcontroller
#define pidcontroller
#include <parameters.h>
#include <math.h>

/*****************************************************************************/
/* UPDATE the Control Variable (CV)                                          */
/* for heaters, sqrt of CV rescaled to 100% is used for linear power control */
/*****************************************************************************/
void pid_updateCV (channelMap ch, boolean isPower = true)
{
   float err = PID[ch].SP - PID[ch].PV;   // error = set value - measured
   PID[ch].I += (err * PID[ch].Ki);       // integral value 
   float P = err * PID[ch].kP;            // proportional value
   float D = PID[ch].kD * (PID[ch].PV - PID[ch].PVold); // derivative value
   PID[ch].CV = P + PID[ch].I + D;        // new control variable
   if (PID[ch].CV > 100) 
   {
      PID[ch].CV = 100;// limit output to 100%
   }
   if (isPower)
   {
      PID[ch].CV = sqrt(PID[ch].CV) * 10;  // convert power to voltage
   }
}

#endif
