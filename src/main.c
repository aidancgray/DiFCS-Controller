#include <main.h>
#include <io.h>
#include <parameters.h>
#include <monitor.h>
#include <control.h>
#include <serialport.h>
#include <pid.h>
#include <eventTimer.h>
#include <commandHandler.c>

#define DEBUG_1

void main()
{
   IO_init();                    // set up IO 
   params_init();                // load parameters
   monitor_init();               // initialize internal ADC for voltage and current telemetry
   control_init();               // initialize the output control DACs
   serial_init();                // setup the serial port
   event_timer_init();
//!   setup_wdt(WDT_512MS);
   enable_interrupts(GLOBAL);
   
   while(TRUE)
   {
//!      restart_wdt();
      internal_monitor_task();   //Update monitored voltage and currents etc
      sensor_monitor_task();     //get magnetoresistive sensor data
      control_task();            //output control
      serial_task();             //serial port 
      
      #ifdef DEBUG_1
      static int8 ch = 0;
      fprintf(SERIAL, "#D0,CNT,%u,%.0f,%.0f\n", ch+1, adcVals[ch].sinCounts, adcVals[ch].cosCounts);
      fprintf(SERIAL, "#D0,POS,%u,%3.3f\n", ch+1, adcVals[ch].pReal);
      if ( dacVals[ch].invV ) fprintf(SERIAL, "#D0,OUT,%u,-,%Lu\n", ch+1, dacVals[ch].val);
      else                    fprintf(SERIAL, "#D0,OUT,%u,+,%Lu\n", ch+1, dacVals[ch].val);
      ch = !ch;
      #endif
      
      command_handler_task();    //execute commands
      
      delay_ms(250);
   }
}
