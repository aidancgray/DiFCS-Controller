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

void main()
{
    int16 loopDelay = 100;
    int16 debugCounter = 0;
   
    char pBuff[100];
    
    IO_init();                    // set up IO 
    params_init();                // load parameters
    monitor_init();               // initialize internal ADC for voltage and current telemetry
    control_init();               // initialize the output control DACs
    serial_init();                // setup the serial port
    event_timer_init();
//!    setup_wdt(WDT_512MS);
    enable_interrupts(GLOBAL);
    
    while(TRUE)
    {
//!        restart_wdt();
        
        if (adcVals[chX].homeFlag) home_axis(chX);
        if (adcVals[chY].homeFlag) home_axis(chY);
                
        internal_monitor_task();
        sensor_monitor_task();
        control_task();
        serial_task();     
        command_handler_task();
        
        #ifdef DEBUG_1
        pBuff[0] = '\0';
        static int8 ch = 0;
        sprintf(pBuff+strlen(pBuff), "$,%u,%.0f,%.0f;", ch+1, adcVals[ch].sinCounts, adcVals[ch].cosCounts);
        ch = !ch;
        serial_out(pBuff);
        #endif
      
        delay_ms(loopDelay);
    }
}
