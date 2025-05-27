#include <main.h>
#include <io.h>
#include <parameters.h>
#include <monitor.h>
//!#include <control.h>
#include <serialport.h>
//!#include <pid.h>
#include <eventTimer.h>
#include <commandHandler.c>

void main()
{
   IO_init();                    // set up IO 
   params_init();                // load parameters
   monitor_init();               // initialize internal ADC for voltage and current telemetry
//!   control_init();               // initialize the output control DACs
   serial_init();                // setup the serial port
   event_timer_init();
//!   setup_wdt(WDT_512MS);
   enable_interrupts(GLOBAL);

   while(TRUE)
   {
//!      restart_wdt();
      internal_monitor_task();   //Update monitored voltage and currents etc
      sensor_monitor_task();     //get magnetoresistive sensor data
//!      control_task();            //output control
      serial_task();             //serial port 
      command_handler_task();    //execute commands
//!      signed int32 x_sin = (signed int32)adcVals[0].sinCounts;
//!      signed int32 x_cos = (signed int32)adcVals[0].cosCounts;
      
//!      signed int32 y_sin = (signed int32)adcVals[1].sinCounts;
//!      signed int32 y_cos = (signed int32)adcVals[1].cosCounts;
      
//!      fprintf(SERIAL, "x_sin=%Ld ; x_cos=%Ld ; y_sin=%Ld ; y_cos=%Ld\n",x_sin,x_cos,y_sin,y_cos);
      delay_ms(500);
   }
}
