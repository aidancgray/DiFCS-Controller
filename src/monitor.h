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

#define BUFFER_SIZE 10
typedef struct {
   int in;
   int out;
   int buff[BUFFER_SIZE];
} buffer;

buffer sinQ_x;
buffer cosQ_x;
buffer sinQ_y;
buffer cosQ_y;

#define incin(buff) ((buff->in==(BUFFER_SIZE-1))?0:buff->in+1)
#define incout(buff) ((buff->out==(BUFFER_SIZE-1))?0:buff->out+1)
#define isempty(buff) (buff->in==buff->out)
#define hasdata(buff) (buff->in!=buff->out)
#define isfull(buff) (incin(buff)==buff->out)

#define tobuff(bname,c) { bname->buff[bname->in]=c;\
   bname->in=incin(bname);\
   if (bname->in==bname->out) bname->out=incout(bname);\
   }
#define frombuff(bname) (btemp##bname=bname->buff[bname->out],\
   bname->out=incout(bname), \
   btemp##bname)
#define clrbuff(buff) buff->in=buff->out=0

struct sensorMonitorData
{
   boolean dataReady;
   boolean adcBusy;
   buffer* sinQ;
   buffer* cosQ;
   signed int32 avgSin;
   signed int32 avgCos;
} smData[2] = 
{
   {false, false, &sinQ_x, &cosQ_x, 0, 0},
   {false, false, &sinQ_y, &cosQ_y, 0, 0}
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
void sensor_process_data(int8 ch)//, signed int32 sinRawCounts, signed int32 cosRawCounts)
{
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

signed int8 compar(signed int32 *arg1,signed int32 *arg2);

/*****************************************************************************/
/* Interquartile Mean Ring Buffer                                            */
/* Filters the ADC data to remove spurious readings                          */
/*****************************************************************************/
void iqm_ring_buffer(int8 ch, signed int32 sinCnts, signed int32 cosCnts)
{
   /* 
   -  copy data out to IQM buffer
   -  sort IQM buffer
   -  average values from middle quartile
   **************************************/
   signed int32 iqmBufSin[BUFFER_SIZE];
   signed int32 iqmBufCos[BUFFER_SIZE];
   signed int32 sumSin=0;
   signed int32 sumCos=0;
   
   tobuff(smData[ch].sinQ, sinCnts); // push new data into queues
   tobuff(smData[ch].cosQ, cosCnts);
   
   // copy queue contents out to buffer for qsorting
   for (int8 i=0; i<BUFFER_SIZE; i++){
      iqmBufSin[i] = smData[ch].sinQ->buff[i];   
      iqmBufCos[i] = smData[ch].cosQ->buff[i];   
   }
   
   qsort(iqmBufSin, BUFFER_SIZE, sizeof(*iqmBufSin), compar);
//!   qsort(iqmBufCos, BUFFER_SIZE, sizeof(*iqmBufCos), compar);
   
   for (int8 i=1; i<BUFFER_SIZE-1; i++){
      sumSin+=iqmBufSin[i];
      sumCos+=iqmBufCos[i];
   }
   smData[ch].avgSin = sumSin >> 3;
   smData[ch].avgCos = sumCos >> 3;
}

signed int8 compar(signed int32 *arg1,signed int32 *arg2)  {
   if (* (signed int32 *) arg1 < (* (signed int32 *) arg2)) return -1;
   else if (* (signed int32 *) arg1 == (* (signed int32 *) arg2)) return 0;
   else return 1;
}

/*****************************************************************************/
/* SENSOR MONITOR task - gets magnetoresistive sensor counts                 */
/* gets two values each time the task is run                                 */
/*****************************************************************************/
void sensor_monitor_task()
{
   static int8 ch = 0;
   signed int32 sinNew = 0;
   signed int32 cosNew = 0;
   
   if ( (!smData[ch].adcBusy) && smData[ch].dataReady ){
      smData[ch].adcBusy = true;
      
      sinNew = ads_read_data(ch*2);
      cosNew = ads_read_data(ch*2+1);      
      
      iqm_ring_buffer(ch, sinNew, cosNew);
      
      sensor_process_data(ch);
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
   
   for(int8 i = 0; i < BUFFER_SIZE; i++){
      tobuff(smData[0].sinQ, 0);
      tobuff(smData[0].cosQ, 0);
      tobuff(smData[1].sinQ, 0);
      tobuff(smData[1].cosQ, 0);      
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
