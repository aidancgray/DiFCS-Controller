#include <main-adc-test.h>
#include <io.h>
#include <parameters.h>
#include <monitor.h>
#include <serialport.h>
#include <eventTimer.h>

#define DEBUG_1

void main()
{
    int16 loopDelay = 100;
    int16 debugCounter = 0;
   
    char pBuff[100];
    
    IO_init();                    // set up IO 
    params_init();                // load parameters
    monitor_init();               // initialize internal ADC for voltage and current telemetry
    serial_init();                // setup the serial port
    event_timer_init();
    enable_interrupts(GLOBAL);
    
    while(TRUE)
    {
        // internal_monitor_task();   //Update monitored voltage and currents etc
        // sensor_monitor_task();     //get magnetoresistive sensor data
        // serial_task();             
        // command_handler_task();    //execute commands
        
        // #######################################################################################
        #ifdef DEBUG_1
        
        pBuff[0] = '\0';
        sensor_monitor_task_test();     //get magnetoresistive sensor data
        if (debugCounter >= 0){
            static int8 ch = 0;
            sprintf(pBuff+strlen(pBuff), "%u,%.0f,%.0f;", ch+1, adcVals[ch].sinCounts, adcVals[ch].cosCounts);
            ch = !ch;
            debugCounter = 0;
        }else debugCounter++;
        serial_out(pBuff);
        
        #endif
        // #######################################################################################

        delay_ms(loopDelay);
    }
}
