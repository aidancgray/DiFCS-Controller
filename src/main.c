#include <main.h>
#include <io.h>
#include <parameters.h>
#include <monitor.h>
#include <control.h>
#include <serialport.h>
#include <pid.h>
#include <eventTimer.h>
#include <commandHandler.c>

void main()
{
   IO_init();                    // set up IO 
   params_init();                // load parameters
   monitor_init();               // initialize internal ADC for voltage and current telemetry
   control_init();               // initialize the output control DACs
   output_high(EN_EXC);
   serial_init();                // setup the serial port
//!   event_timer_init();
//!   setup_wdt(WDT_512MS);
   enable_interrupts(GLOBAL);
   
   while(TRUE)
   {
//!      restart_wdt();
      internal_monitor_task();   //Update monitored voltage and currents etc
//!      sensor_monitor_task();     //get magnetoresistive sensor data
      control_task();            //output control
      serial_task();             //serial port 
      
//!      adcVals[0].sinRaw = ads_read_data(0);
//!      adcVals[0].cosRaw = ads_read_data(1);
//!      adcVals[1].sinRaw = ads_read_data(2);
//!      adcVals[1].cosRaw = ads_read_data(3);
//!      
//!      signed int32 xs = adcVals[0].sinRaw;
//!      signed int32 xc = adcVals[0].cosRaw;
//!      
//!      signed int32 ys = adcVals[1].sinRaw;
//!      signed int32 yc = adcVals[1].cosRaw;
      
//!      fprintf(SERIAL, "y= %Ld, %Ld | x= %Ld, %Ld\r\n", ys, yc, xs, xc);
      
      signed int32 xs = ads_read_data(0);
      fprintf(SERIAL, "%Ld\r\n", xs);
      
      signed int32 xc = ads_read_data(1);
      fprintf(SERIAL, "%Ld\r\n", xc);
      
      signed int32 ys = ads_read_data(2);
      fprintf(SERIAL, "%Ld\r\n", ys);
      
      signed int32 yc = ads_read_data(3);
      fprintf(SERIAL, "%Ld\r\n", yc);
      
      command_handler_task();    //execute commands
      
      delay_ms(500);
   }
}
