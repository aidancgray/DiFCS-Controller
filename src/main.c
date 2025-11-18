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
//!      fprintf(SERIAL, "#D0,%u,%.0f,%.0f,SUCCESS\n", ch+1, adcVals[ch].sinCounts, adcVals[ch].cosCounts);
      fprintf(SERIAL, "#D0,%u,%3.3f\n", ch+1, adcVals[ch].pReal);
      fprintf(SERIAL, "#D0,OP,%u,%Lu\n", ch+1, dacVals[ch]);
      ch = !ch;
      #endif
      
      command_handler_task();    //execute commands
      
      delay_ms(250);
   }
}
