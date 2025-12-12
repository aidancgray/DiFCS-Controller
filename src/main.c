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
    int16 loopDelay = 1000;
    int16 controlCounter = 0;
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
        pBuff[0] = '\0';
        
//!        if (adcVals[chX].homeFlag) home_axis(chX);
//!        if (adcVals[chY].homeFlag) home_axis(chY);
                
        internal_monitor_task();   //Update monitored voltage and currents etc
        sensor_monitor_task();     //get magnetoresistive sensor data
      
        if (controlCounter >= 2){
            control_task();
            controlCounter = 0;
        }else controlCounter++;
      
        serial_task();             
      
        #ifdef DEBUG_1
        if (debugCounter >= 0){
            static int8 ch = 0;
            sprintf(pBuff+strlen(pBuff), "CNT,%u,%.0f,%.0f;", ch+1, adcVals[ch].sinCounts, adcVals[ch].cosCounts);
            sprintf(pBuff+strlen(pBuff), "POS,%u,%3.3f;", ch+1, adcVals[ch].pReal);
            if ( dacVals[ch].invV ) sprintf(pBuff+strlen(pBuff), "OUT,%u,-,%Lu;", ch+1, dacVals[ch].ipVal);
            else                    sprintf(pBuff+strlen(pBuff), "OUT,%u,+,%Lu;", ch+1, dacVals[ch].ipVal);
            ch = !ch;
            debugCounter = 0;
        }else debugCounter++;
        fprintf(ICD_STREAM, "%s\n", pBuff);
        #endif
      
        command_handler_task();    //execute commands
      
        delay_ms(loopDelay);
    }
}
