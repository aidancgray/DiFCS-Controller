#ifndef control
#define control
#include <parameters.h>

#pin_select U1TX = TX_PIN
#pin_select U1RX = RX_PIN
#pin_select SCK1=PIN_C3
#pin_select SDO1=PIN_C2
#use spi (MASTER, SPI1, BAUD=1000000, MODE=0, STREAM=SPI_ctrl)

#define DACfullScale 655.36 // full scale/100 because CV = %

/*****************************************************************************/
/*  Nano DAC's have a 16bit serial shift register. Data is sync'ed to output */ 
/*****************************************************************************/
void set_nanoDAC_outputs(channelMap ch) 
{
   // calculate DAC CONTROL VARIABLE value
   unsigned int16 txData = (unsigned int16)(PID[(int)ch].CV * DACfullScale);

   // shift 16 bits of data
   spi_xfer(SPI_ctrl, txData, 24);

   // use channel map to decide which outputs channels to update and enable
   // note that one input channel can control multiple control channels
   // strobe _sync low to push data to the outputs
   if (chMap[0] == ch) 
   {
      output_low(_SYNC_A);
      output_high(EN_A);
   }
   if (chMap[1] == ch) 
   {
      output_low(_SYNC_B);
      output_high(EN_B);
   }
   if (chMap[2] == ch) 
   {
      output_low(_SYNC_C);
      output_high(EN_C);
   }
   if (chMap[3] == ch) 
   {
      output_low(_SYNC_D);
      output_high(EN_D);
   }
   
   output_high(_SYNC_A);
   output_high(_SYNC_B);
   output_high(_SYNC_C);
   output_high(_SYNC_D);
   
   // use channel map to disable unused channels 
   if (chMap[0] == op_disabled) output_low(EN_A);
   if (chMap[1] == op_disabled) output_low(EN_B);
   if (chMap[2] == op_disabled) output_low(EN_C);
   if (chMap[3] == op_disabled) output_low(EN_D);  
}

/*****************************************************************************/
/*  control task sets up the outputs channels                                */ 
/*****************************************************************************/
void control_task()
{
   static int state = 0;
   
   switch (state)
   {
      case 0:       
         set_nanoDAC_outputs(ch0);
         state = 1;
      break;
      case 1:
         set_nanoDAC_outputs(ch1);
         state = 2;
      break;
      case 2:
         set_nanoDAC_outputs(ch2);
         state = 3;
      break;
      default:
         set_nanoDAC_outputs(ch3);
         state = 0;
      break;
   }
}

/*****************************************************************************/
/*  control task sets up the outputs channels                                */ 
/*****************************************************************************/
void control_init()
{
   spi_xfer(SPI_ctrl, 0, 24);
   output_low(_SYNC_A);
   output_low(_SYNC_B);
   output_low(_SYNC_C);
   output_low(_SYNC_D);
   output_high(_SYNC_A);
   output_high(_SYNC_B);
   output_high(_SYNC_C);
   output_high(_SYNC_D);
   output_low(EN_A);
   output_low(EN_B);
   output_low(EN_C);
   output_low(EN_D);  
}
#endif
