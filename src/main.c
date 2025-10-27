#include <main.h>
#include <io.h>
#include <parameters.h>
#include <monitor.h>
#include <control.h>
#include <serialport.h>
#include <pid.h>
#include <eventTimer.h>
#include <commandHandler.c>


//!#define DEBUG_1
//!#define DEBUG_2

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
      fprintf(SERIAL, "#D0,1,%.0f,%.0f,SUCCESS\n", adcVals[0].sinCounts, adcVals[0].cosCounts);
      fprintf(SERIAL, "#D0,2,%.0f,%.0f,SUCCESS\n", adcVals[1].sinCounts, adcVals[1].cosCounts);
      fprintf(SERIAL, "#D0,1,%3.3f,SUCCESS\n", adcVals[0].pReal);
      fprintf(SERIAL, "#D0,2,%3.3f,SUCCESS\n", adcVals[1].pReal);
      #endif
      
      #ifdef DEBUG_2
      set_adc_channel(vMon3V6X);
      delay_ms(10);
      int16 mon3V6X = read_adc(ADC_READ_ONLY);
      delay_ms(10);
      read_adc(ADC_START_ONLY);
      delay_ms(10);
      fprintf(SERIAL, "%Ld\n", mon3V6X);
      #endif
      
      command_handler_task();    //execute commands
      
      delay_ms(200);
   }
}
