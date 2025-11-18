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
   }
   else {
      if (chMap[0] == ch) output_high(INV_HVX);
      if (chMap[1] == ch) output_high(INV_HVY);
   }
}

/*****************************************************************************/
/*  Nano DAC's have a 16bit serial shift register. Data is sync'ed to output */ 
/*****************************************************************************/
void set_nanoDAC_outputs(channelMap ch){
   unsigned int16 txData = 0;
   
   // use manualOutputValues if channel is manual mode
   // else, use PID control variable
   if (chMode[ch] == MANUAL){
      if ( manualOutputValues[ch] < 0) invert_voltage(ch, TRUE); 
      else                             invert_voltage(ch, FALSE);
      
      txData = (unsigned int16)(abs(manualOutputValues[ch]) * DACfullScale);
   }
   else {
      pid_task(ch);
      if ( PID[(int)ch].CV < 0 ) invert_voltage(ch, TRUE); 
      else                       invert_voltage(ch, FALSE);
      
      txData = (unsigned int16)(abs(PID[(int)ch].CV) * DACfullScale);
   }
   
   dacVals[ch] = txData;
   
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
