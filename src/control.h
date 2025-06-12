#ifndef control
#define control
#include <parameters.h>
#include <stdbool.h>

//!#pin_select U1TX = TX_PIN
//!#pin_select U1RX = RX_PIN
#pin_select SCK1=PIN_C3
#pin_select SDO1=PIN_C2
#use spi (MASTER, SPI1, BAUD=100000, MODE=0, STREAM=SPI_ctrl)

#define DACfullScale 655.36 // full scale/100 because CV = %

//!typedef enum {pid, off, manual}OpModeMap_t;
//!   
//!typedef struct {
//!   OpModeMap_t chMap;
//!   int16 syncPin;
//!   int16 enPin;
//!   int16 man_CV;
//!   PID_Ctrl_t* hPID;
//!   }control_ch_t;
//!   
//!extern control_ch_t hPzo[];   

/*****************************************************************************/
/*  Nano DAC's have a 16bit serial shift register. Data is sync'ed to output */ 
/*****************************************************************************/
//!void set_nanoDAC_output(control_ch_t *hPzo) 
//!{
//!   unsigned int16 txData = 0;
//!   if (hPzo->chMap == manual)
//!   {
//!      txData = hPzo->man_CV;
//!      output_high(hPzo->enPin);
//!   }
//!   else if (hPzo->chMap == pid)
//!   {
//!      txData = hPzo->hPID->CV;
//!      output_high(hPzo->enPin);
//!   }
//!   else
//!   {
//!      output_low(hHtr->enPin);
//!   }   
//!   output_high(hPzo->syncPin);
//!   delay_ms(1);
//!   spi_xfer(SPI_ctrl, (unsigned int32)txData, 24);
//!   delay_ms(1);
//!   output_low(hPzo->syncPin);
//!   delay_ms(1);
//!   output_high(hPzo->syncPin);
//!}

void set_nanoDAC_outputs(channelMap ch)
{
   unsigned int16 txData = 0;

   // use manualOutputValues if channel is manual mode
   // else, use PID control variable
   if (chMode[ch] == MANUAL)  txData = (unsigned int16)(manualOutputValues[ch] * DACfullScale);
   else                       txData = (unsigned int16)(PID[(int)ch].CV * DACfullScale);
   
   // shift 16 bits of data
   spi_xfer(SPI_ctrl, txData, 24);
   
   // use channel map to decide which outputs channels to update
   // strobe _sync low to push data to the outputs
   output_high(_SYNC_X);
   output_high(_SYNC_Y);
   delay_ms(1);
   
   if (chMap[0] == ch) output_low(_SYNC_X);
   if (chMap[1] == ch) output_low(_SYNC_Y);
   
   delay_ms(1);
   output_high(_SYNC_X);
   output_high(_SYNC_Y);
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
         set_nanoDAC_outputs(chX);
         state = 1;
      break;
      default:
         set_nanoDAC_outputs(chY);
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
   output_low(_SYNC_X);
   output_low(_SYNC_Y);
   output_high(_SYNC_X);
   output_high(_SYNC_Y);
}
#endif
