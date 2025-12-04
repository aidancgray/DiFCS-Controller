#ifndef monitor
#define monitor
#include <io.h>
#include <parameters.h>
#include <ADS1220.h>
#include <math.h>

#define sensorSampleRate 50 // 50mS delay between each sensor sample = 100ms cadence 

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

#define BUFFER_SIZE 5

signed int32 sQ_x[BUFFER_SIZE];
signed int32 cQ_x[BUFFER_SIZE];
signed int32 sQ_y[BUFFER_SIZE];
signed int32 cQ_y[BUFFER_SIZE];
int sIn_x=0;
int cIn_x=0;
int sIn_y=0;
int cIn_y=0;

struct sensorMonitorData
{
   boolean dataReady;
   boolean adcBusy;
   signed int32 avgSin;
   signed int32 avgCos;
} smData[2] = 
{
   {false, false, 0, 0},
   {false, false, 0, 0}
};

void push(signed int32* buff, int* idx, signed int32 newData) {
   buff[*idx]=newData;
   *idx=(*idx+1) % BUFFER_SIZE;
}

/*****************************************************************************/
/* INTERNAL MONITOR task - gets voltages                                     */
/* gets one value each time the task is run                                  */
/*****************************************************************************/
void internal_monitor_task(){
   static int8 state = 0;
   if (adc_done()){
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
void sensor_process_data(int8 ch){
   adcVals[ch].sinLast = adcVals[ch].sinCounts;
   adcVals[ch].cosLast = adcVals[ch].cosCounts;
   
   adcVals[ch].sinRaw = (float)smData[ch].avgSin;
   adcVals[ch].cosRaw = (float)smData[ch].avgCos;
   
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
   if (ch == chX) adcVals[ch].pReal *= -1;
   PID[ch].PVold = PID[ch].PV;
   PID[ch].PV = adcVals[ch].pReal;
}  

/*****************************************************************************/
/* SENSOR MONITOR INTERRUPT TASK                                             */
/* starts conversion of the other two ADCs                                   */
/*****************************************************************************/
void sensor_monitor_interrupt_task(){
   static int8 ch = 0;
   
   if (!smData[ch].adcBusy){
      smData[ch].adcBusy = true;
      
      ads_start_conv_block(ch);
      ch = !ch;
      
      smData[!ch].dataReady = true;
      smData[!ch].adcBusy = false;
   }
}

/*****************************************************************************/
/* Interquartile Mean Ring Buffer                                            */
/* Filters the ADC data to remove spurious readings                          */
/*****************************************************************************/
void iqm_ring_buffer(int8 ch, signed int32 sinCnts, signed int32 cosCnts){
   signed int32* sQ_ch;
   signed int32* cQ_ch;
   int* sIn_ch;
   int* cIn_ch;

   signed int32 sumSin=0;
   signed int32 sumCos=0;
   
   signed int32 sinMax=0;
   signed int32 sinMin=0;
   signed int32 cosMax=0;
   signed int32 cosMin=0;
   
   if (ch==0){
      sQ_ch = sQ_x;
      cQ_ch = cQ_x;
      sIn_ch = &sIn_x;
      cIn_ch = &cIn_x;
   }
   else if (ch==1){
      sQ_ch = sQ_y;
      cQ_ch = cQ_y;
      sIn_ch = &sIn_y;
      cIn_ch = &cIn_y;
   }

   push(sQ_ch, sIn_ch, sinCnts); // push new data into queues
   push(cQ_ch, cIn_ch, cosCnts);
   
   // MAX and MIN of the rolling buffer initial vals
   sinMax=sQ_ch[0];
   sinMin=sQ_ch[0];
   cosMax=cQ_ch[0];
   cosMin=cQ_ch[0];
   
   // step thru buffer, adding up all vals and finding MAX and MIN
   for (int i=1; i<BUFFER_SIZE; i++){
      (sinMax < sQ_ch[i]) ? (sinMax=sQ_ch[i]) : (0);
      (sinMin > sQ_ch[i]) ? (sinMin=sQ_ch[i]) : (0);
      (cosMax < cQ_ch[i]) ? (cosMax=cQ_ch[i]) : (0);
      (cosMin > cQ_ch[i]) ? (cosMin=cQ_ch[i]) : (0);
      sumSin+=sQ_ch[i];
      sumCos+=cQ_ch[i];
   }
   
   // subtract MAX and MIN from sum
   sumSin-=(sinMax+sinMin);
   sumCos-=(cosMax+cosMin);
   
   // calc AVG
   smData[ch].avgSin = sumSin / (BUFFER_SIZE-2);
   smData[ch].avgCos = sumCos / (BUFFER_SIZE-2);
}

/*****************************************************************************/
/* SENSOR MONITOR task - gets magnetoresistive sensor counts                 */
/* gets two values each time the task is run                                 */
/*****************************************************************************/
void sensor_monitor_task(){
   static int8 ch = 0;
   signed int32 sinNew = 0;
   signed int32 cosNew = 0;
   
   if ( (!smData[ch].adcBusy) && smData[ch].dataReady ){
      smData[ch].adcBusy = true;
      
      sinNew = ads_read_data(ch*2);
      cosNew = ads_read_data(ch*2+1);      
      
      if (adcFilter){
         iqm_ring_buffer(ch, sinNew, cosNew);
      }
      else{
         smData[ch].avgSin = sinNew;
         smData[ch].avgCos = cosNew;
      }
      
      sensor_process_data(ch);
      ch = !ch;
      
      smData[!ch].dataReady = false;
      smData[!ch].adcBusy = false;
   }
}
/*****************************************************************************/
/* INITIALIZE ADC's                                                          */
/*****************************************************************************/
void setup_external_ADCs(){
   unsigned int8 rc0=0;
   unsigned int8 rc1=0;
   unsigned int8 rc2=0;
   unsigned int8 rc3=0;
   
   for (int ch = 0; ch < 4; ch++){
      rc0=reg0config;
      rc1=reg1config;
      rc2=reg2config;
      rc3=reg3config;
      
      ADS1220init(ch, rc0, rc1, rc2, rc3);
      delay_ms(100);
   }
   
   if (adcFilter){
      for (int i = 0; i < BUFFER_SIZE; i++){
         ads_start_conv_all();
         delay_ms(50);
         push(sQ_x, &sIn_x, ads_read_data(0));
         push(cQ_x, &cIn_x, ads_read_data(1));      
         push(sQ_y, &sIn_y, ads_read_data(2));
         push(cQ_y, &cIn_y, ads_read_data(3));      
      }
   }
}

/*****************************************************************************/
/* INITIALIZE MONITOR -- sets up ADC channels                                */
/*****************************************************************************/
void monitor_init(){
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
