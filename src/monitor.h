#ifndef monitor
#define monitor
#include <io.h>
#include <parameters.h>
#include <ADS1220.h>
#include <math.h>

#define sensorSampleRate 50 // 50mS delay between each sensor sample = 200ms cadence 

#define maxCount 16777216.0   // ADC max count
#define vRef 2.048            // ADC vRef

#define sc sensorCal

#define vMonN15   6
#define vMon200   16
#define vMon5V6   24
#define vMon5VA   25
#define vMon3V6X  17
#define vMon3V3A  27
#define vMon3V3D  26

struct sensorMonitorData
{
   boolean dataReady;
   boolean adcBusy;
} smData[2] = 
{
   {false, false},
   {false, false}
};

/*****************************************************************************/
/* INTERNAL MONITOR task - gets voltages                                     */
/* gets one value each time the task is run                                  */
/*****************************************************************************/
void internal_monitor_task()
{
   static int8 state = 0;
   if (adc_done())
   {
      switch (state)
      {
         case 0:     // vMonN15
            monitorVals.vN15 = read_adc(ADC_READ_ONLY) * monCal.vN15[1] + monCal.vN15[0];
            set_adc_channel(vMon200);
            delay_ms(10);
            read_adc(ADC_START_ONLY);
            state = 1;
         break;
         
         case 1:     // vMon200
            monitorVals.v200 = read_adc(ADC_READ_ONLY) * monCal.v200[1] + monCal.v200[0];
            set_adc_channel(vMon5V6);
            delay_ms(10);
            read_adc(ADC_START_ONLY);
            state = 2;
         break;
         
         case 2:     // vMon5V6
            monitorVals.v5V6 = read_adc(ADC_READ_ONLY) * monCal.v5V6[1] + monCal.v5V6[0];
            set_adc_channel(vMon5VA);
            delay_ms(10);
            read_adc(ADC_START_ONLY);
            state = 3;
         break;
         
         case 3:     // vMon5VA
            monitorVals.v5VA = read_adc(ADC_READ_ONLY) * monCal.v5VA[1] + monCal.v5VA[0];
            set_adc_channel(vMon3V6X);
            delay_ms(10);
            read_adc(ADC_START_ONLY);
            state = 4;
         break;
         
         case 4:     // vMon3V6X
            monitorVals.v3V6X = read_adc(ADC_READ_ONLY) * monCal.v3V6X[1] + monCal.v3V6X[0];
            set_adc_channel(vMon3V3A);
            delay_ms(10);
            read_adc(ADC_START_ONLY);
            state = 5;
         break;
         
         case 5:     // vMon3V3A
            monitorVals.v3V3A = read_adc(ADC_READ_ONLY) * monCal.v3V3A[1] + monCal.v3V3A[0];
            set_adc_channel(vMon3V3D);
            delay_ms(10);
            read_adc(ADC_START_ONLY);
            state = 6;
         break;
         
         case 6:    // vMon3V3D
            monitorVals.v3V3D = read_adc(ADC_READ_ONLY) * monCal.v3V3D[1] + monCal.v3V3D[0];
            set_adc_channel(vMonN15);
            read_adc(ADC_START_ONLY);
            state = 0;
         break;   
      }
   }
}
/*****************************************************************************/
/* PROCESS ADC SENSOR DATA                                                   */
/*****************************************************************************/
void sensor_process_data(int8 ch, signed int32 sinRawCounts, signed int32 cosRawCounts)
{
   adcVals[ch].sinLast = adcVals[ch].sinCounts;
   adcVals[ch].cosLast = adcVals[ch].cosCounts;
   
   adcVals[ch].sinRaw = (float)sinRawCounts;
   adcVals[ch].cosRaw = (float)cosRawCounts;
   
   adcVals[ch].sinCounts = adcVals[ch].sinRaw * adcCal[ch].sinGain + adcCal[ch].sinOS;
   adcVals[ch].cosCounts = adcVals[ch].cosRaw * adcCal[ch].cosGain + adcCal[ch].cosOS;
   float pTmp = 0;
   float polePitch = magPP;
   
   if (adcVals[ch].cosCounts < 0){
      if ((adcVals[ch].sinCounts > 0) && (adcVals[ch].sinLast < 0)){
         adcVals[ch].npoles--;
      }
      else if ((adcVals[ch].sinCounts < 0) && (adcVals[ch].sinLast > 0)){
         adcVals[ch].npoles++;
      }
   }
   
   adcVals[ch].p0 = polePitch * ( 1000 / (2*PI) ) * atan2( (float)adcVals[ch].sinCounts, (float)adcVals[ch].cosCounts );
   pTmp = adcVals[ch].p0 + ( 1000 * polePitch * (float)adcVals[ch].npoles );
   
   adcVals[ch].pReal = (pow(pTmp*sc[ch].c5,5) + pow(pTmp*sc[ch].c4,4) + pow(pTmp*sc[ch].c3,3) + pow(pTmp*sc[ch].c2,2) + pTmp*sc[ch].c1 + sc[ch].c0);
   
   PID[ch].PVold = PID[ch].PV;
   PID[ch].PV = adcVals[ch].pReal;
}  

/*****************************************************************************/
/* SENSOR MONITOR INTERRUPT TASK                                             */
/* starts conversion of the other two ADCs                                   */
/*****************************************************************************/
void sensor_monitor_interrupt_task()
{
   static int8 ch = 0;
   
   if (!smData[ch].adcBusy)
   {
      smData[ch].adcBusy = true;
      
      ads_start_conv_block(ch);
      ch = !ch;
      
      smData[!ch].dataReady = true;
      smData[!ch].adcBusy = false;
   }
}


/*****************************************************************************/
/* SENSOR MONITOR task - gets magnetoresistive sensor counts                 */
/* gets two values each time the task is run                                  */
/*****************************************************************************/
void sensor_monitor_task()
{
   static int8 ch = 0;
   signed int32 sinRaw = 0;
   signed int32 cosRaw = 0;
   
   if ( (!smData[ch].adcBusy) && smData[ch].dataReady ){
      smData[ch].adcBusy = true;
      
      sinRaw = ads_read_data(ch*2);
      cosRaw = ads_read_data(ch*2+1);      
      
      sensor_process_data(ch, sinRaw, cosRaw);
      ch = !ch;
      
      smData[!ch].dataReady = false;
      smData[!ch].adcBusy = false;
   }
}
/*****************************************************************************/
/* INITIALIZE ADC's                                                          */
/*****************************************************************************/
void setup_external_ADCs()
{
   unsigned int8 rc0=0;
   unsigned int8 rc1=0;
   unsigned int8 rc2=0;
   unsigned int8 rc3=0;
   
   for(int ch = 0; ch < 4; ch++)
   {
      rc0=reg0config;
      rc1=reg1config;
      rc2=reg2config;
      rc3=reg3config;
      
      ADS1220init(ch, rc0, rc1, rc2, rc3);
      delay_ms(1);
   }   
}

/*****************************************************************************/
/* INITIALIZE MONITOR -- sets up ADC channels                                */
/*****************************************************************************/
void monitor_init()
{
   setup_adc_ports(sAN6 | sAN16 | sAN17 | sAN24 | sAN25 | sAN26 | sAN27, NO_ANALOGS_P2, VSS_VDD);
   setup_adc(ADC_CLOCK_INTERNAL | ADC_TAD_MUL_255 | ADC_LEGACY_MODE | ADC_THRESHOLD_INT_DISABLED);
   output_high(EN_EXC);
   set_adc_channel(vMonN15); // internal MUX
   delay_ms(10);
   read_adc(ADC_START_ONLY);
   setup_external_ADCs();
   intTimeoutReg = sensorSampleRate;
}

#endif
