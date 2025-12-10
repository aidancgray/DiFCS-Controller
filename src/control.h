#ifndef control
#define control
#include <parameters.h>
#include <pid.h>
#include <stdbool.h>

#pin_select SCK1=PIN_C3
#pin_select SDO1=PIN_C2
#use spi (MASTER, SPI1, BAUD=100000, MODE=2, STREAM=SPI_ctrl)

#define DACfullScale 655.35 // full scale/100 because CV = %

/*****************************************************************************/
/*  Inverts the output voltage ( -180V - 0V )                                */ 
/*****************************************************************************/
void invert_voltage(channelMap ch, BOOLEAN inv_v){
   if ( inv_v ) {
      if (chMap[0] == ch) output_low(INV_HVX);
      if (chMap[1] == ch) output_low(INV_HVY);
      dacVals[ch].invV = TRUE;
   }
   else {
      if (chMap[0] == ch) output_high(INV_HVX);
      if (chMap[1] == ch) output_high(INV_HVY);
      dacVals[ch].invV = FALSE;
   }
}

/*****************************************************************************/
/*  Nano DAC's have a 16bit serial shift register. Data is sync'ed to output */ 
/*****************************************************************************/
void set_nanoDAC_outputs(channelMap ch){
    unsigned int16 txData = 0;
   
    // use dacVals.opPcnt values if channel is manual mode
    // else, use PID control variable
    if (chMode[ch] == MANUAL){
        if ( dacVals[ch].opPcnt < 0) invert_voltage(ch, TRUE); 
        else                         invert_voltage(ch, FALSE);
      
        txData = (unsigned int16)(abs(dacVals[ch].opPcnt) * DACfullScale);
    }
    else {
        pid_task(ch);
        if ( PID[(int)ch].CV < 0 ) invert_voltage(ch, TRUE); 
        else                       invert_voltage(ch, FALSE);
      
        txData = (unsigned int16)(abs(PID[(int)ch].CV) * DACfullScale);
    }
   
    dacVals[ch].ipVal = txData;
   
    // use channel map to decide which outputs channels to update
    // strobe _sync low to push data to the outputs
    if (chMap[0] == ch) output_high(_SYNC_X);
    if (chMap[1] == ch) output_high(_SYNC_Y);
   
    delay_ms(1);
   
    if (chMap[0] == ch) output_low(_SYNC_X);
    if (chMap[1] == ch) output_low(_SYNC_Y);
    delay_ms(1);
   
    // shift 16 bits of data
    spi_xfer(SPI_ctrl, txData, 24);
    delay_ms(1);
   
    if (chMap[0] == ch) output_high(_SYNC_X);
    if (chMap[1] == ch) output_high(_SYNC_Y);
}

/*****************************************************************************/
/*  Axis homing routines and helper functions                                */ 
/*****************************************************************************/
void slew_to_upper_bound(channelMap ch){
    chMode[ch] = MANUAL;        // set channel to manual
    
    while (dacVals[ch].opPcnt < op_upper_bound){
        dacVals[ch].opPcnt += 10;   // increase output by 10%
        set_nanoDAC_outputs(ch);
        delay_ms(500);
    }
    dacVals[ch].opPcnt = op_upper_bound;    // set output to upper bound
    set_nanoDAC_outputs(ch);
    delay_ms(500);
}

void slew_to_lower_bound(channelMap ch){
    chMode[ch] = MANUAL;        // set channel to manual
    
    while (dacVals[ch].opPcnt > op_lower_bound){
        dacVals[ch].opPcnt -= 10;   // decrease output by 10%
        set_nanoDAC_outputs(ch);
        delay_ms(500);
    }
    dacVals[ch].opPcnt = op_lower_bound;    // set output to lower bound
    set_nanoDAC_outputs(ch);
    delay_ms(500);
}

void home_axis(channelMap ch){
    float maxSP = 0;
    float minSP = 0;
    int loops = 3;
    
    // Slew up and down a couple times then set output to 0 to settle the piezo
    slew_to_upper_bound(ch);
    slew_to_lower_bound(ch);
    slew_to_upper_bound(ch);
    slew_to_lower_bound(ch);
    dacVals[ch].opPcnt = 0;
    set_nanoDAC_outputs(ch);
    delay_ms(500);
    
    for (int i=0; i<loops; i++){
        slew_to_upper_bound(ch);
        sensor_monitor_task();
        maxSP+=adcVals[ch].pReal;
        
        slew_to_lower_bound(ch);
        sensor_monitor_task();
        minSP+=adcVals[ch].pReal;
    }
    
    PID[ch].maxSP = maxSP / (float)loops;
    PID[ch].minSP = minSP / (float)loops;
    adcVals[ch].pHome = (maxSP+minSP) / ((float)loops*2);
    adcVals[ch].homeFlag = False;
}

/*****************************************************************************/
/*  control task sets up the outputs channels                                */ 
/*****************************************************************************/
void control_task(){
   static int state = 0;
   
   switch (state)
   {
      case 0:
         set_nanoDAC_outputs(chX);
         state = 1;
      break;
      case 1:
         set_nanoDAC_outputs(chY);
         state = 0;
      break;
   }
}

/*****************************************************************************/
/*  control task sets up the outputs channels                                */ 
/*****************************************************************************/
void control_init(){
   spi_xfer(SPI_ctrl, 0, 24);
   output_low(_SYNC_X);
   output_low(_SYNC_Y);
   output_high(_SYNC_X);
   output_high(_SYNC_Y);
}
#endif
