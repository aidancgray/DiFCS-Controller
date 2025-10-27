#ifndef parameters
#define parameters

//#define use_ext_eeprom

/*****************************************************************************/
/*****************************************************************************/
/* PARAMETERS                                                                */
/* Global parameters used by all routines                                    */
/* These defaults are defined below. On first boot, they will be stored in   */
/* EEProm. On subsequent boots, they will be reloaded from EEProm. Any       */
/* configuation changes will be changed in EEprom, hence will be restored on */
/* subsequent boot. All params start at page boundaries!!                    */
/*****************************************************************************/
/*****************************************************************************/
#include <stddef.h>
#include <stdlib.h>

typedef enum channelMap {chX, chY}; // Piezo channels
typedef enum channelMode {MANUAL, MAGSNS}; // Voltage or Magnetic Sensor Control

/*****************************************************************************/
/* CAT24C04 EEProm --- page size = 16 bytes, Write when LSB=0 Read when LSB=1*/
/*****************************************************************************/
#ifdef use_ext_eeprom
#define EE_Slave_Addr 0xA0
#define writeProtectPin EE_WP
#define pageSize 16
// EEProm is on 12C bus 1
#use i2c(master, fast, sda=PIN_C4, scl=PIN_C3, FORCE_HW, STREAM=EEPROM)
#endif

/*****************************************************************************/
/*****************************************************************************/
/* FUNCTIONS PROTOTYPES START HERE                                           */
/*****************************************************************************/
/*****************************************************************************/
#ifdef use_ext_eeprom
void ee_send_start(unsigned int16 address, BOOLEAN read);
#endif
void ee_save_reg(unsigned int8 *reg, unsigned int16 eeLocation, unsigned int8 length);
void ee_load_reg(unsigned int8 *reg, unsigned int16 eeLocation, unsigned int8 length);
void params_save_to_ee(void);
void params_load_from_ee(void);
void params_init(void);
void params_invalidate(void);

/*****************************************************************************/
/*****************************************************************************/
/* PARAMETERS START HERE. All Params should have an EEPROM address that is   */
/* at the start of an EERPOM page.                                           */ 
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* PID Parameters                                                            */
/*****************************************************************************/
#define pid_params_ee_addr 0
#define pid_params_length 128  // length in bytes
struct pidParams
{
   float kP;     // proportional gain
   float kI;     // integral gain
   float kD;     // derivative gain
   float SP;     // set point-- desired output
   float PV;     // process variable-- measured output
   float PVold;  // oldPV value  
   float CV;     // control variable-- output -- 0 to 100%
   float I;      // integral value
}PID[2] = {
   {0.01,0,0,0,0,0,0,0},
   {0.01,0,0,0,0,0,0,0}
   };

/*****************************************************************************/
/* Output Channel Map                                                        */
/*****************************************************************************/
#define channel_map_ee_addr 128
#define channel_map_length 2  // length in bytes
channelMap chMap[2] = {chX, chY};

/*****************************************************************************/
/* Input Channel Mode                                                        */
/*****************************************************************************/
#define channel_mode_ee_addr 144
#define channel_mode_length 2  // length in bytes
channelMode chMode[2] = {MANUAL, MANUAL};

/*****************************************************************************/
/* ADC Calibration Constants                                                 */
/*****************************************************************************/
#define adc_cal_params_ee_addr 160
#define adc_cal_params_length 32  // length in bytes
struct adcCalParms
{
   float sinOS;
   float cosOS;
   float sinGain;
   float cosGain;
} adcCal[2] = {
                {0,0,1,1},
                {0,0,1,1}
              };

/*****************************************************************************/
/* Sensor Calibration Constants                                              */
/*****************************************************************************/
#define sensor_cal_params_ee_addr 192
#define sensor_cal_params_length 48  // length in bytes
struct sensorCalParams
{
   float c0;
   float c1;
   float c2;
   float c3;
   float c4;
   float c5;
} sensorCal[2] = {
                    {0,1,0,0,0,0},
                    {0,1,0,0,0,0}
                 };

// fifth order polynomial constants

/*****************************************************************************/
/* Monitor Calibration Constants                                             */
/*****************************************************************************/
#define mon_cal_params_ee_addr 240
#define mon_cal_params_length 56  // length in bytes
struct monCalParams
{
   float vN15[2];  // offset and gain
   float v200[2];
   float v5V6[2];
   float v5VA[2];
   float v3V6X[2];
   float v3V3A[2];
   float v3V3D[2];
   
}monCal = {
   {0, -0.02245509}, 
   {0,   0.2127659}, 
   {0, 0.008432515}, 
   {0, 0.008173486}, 
   {0, 0.009655914}, 
   {0, 0.007094828}, 
   {0, 0.007081720}
};

/*****************************************************************************/
/* MANUAL OUTPUT CTRL                                                        */
/*****************************************************************************/
#define man_op_vals_ee_addr 304
#define man_op_vals_length 8
float manualOutputValues[2] = {0,0};

/*****************************************************************************/
/* PARAMS Valid                                                              */
/*****************************************************************************/
#define invalidate 0xFF
#define isValid 0xAA
#define params_valid_ee_addr 320
#define params_valid_length 1  // length in bytes
char paramsValid = isValid;
// if data returns isValid from EEPROM then EEPROM has valid data

/*****************************************************************************/
/*****************************************************************************/
/* VOLATILE PARAMETERS                                                       */
/*****************************************************************************/
/*****************************************************************************/
struct mv
{
   float vN15; // internal voltages
   float v200;
   float v5V6;
   float v5VA;
   float v3V6X;
   float v3V3A;
   float v3V3D;
}monitorVals = {0,0,0,0,0,0,0};

struct adcV
{
   float sinRaw;           // adc raw count data
   float cosRaw;           // adc raw count data
   float sinCounts;        // calibrated counts
   float cosCounts;        // calibrated counts
   float sinLast;          // previous sin data
   float cosLast;          // previous cos data
   float p0;               // mid-pole position in microns
   signed int8  npoles;           // number of poles passed
   float pReal;            // real position in microns
}adcVals[2] = 
{
   {0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0}
};

// used with timeouts
unsigned int32 timeoutReg0 = 0; //All timers are defined in milliseconds, 
unsigned int32 timeoutReg1 = 0; //with a resolution of 10ms
unsigned int32 timeoutReg2 = 0;
unsigned int16 intTimeoutReg = 0; // interrupting use for more precise timing
/*****************************************************************************/
/*****************************************************************************/
/* FUNCTIONS START HERE                                                      */
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* ee_send_start --- sends the start messsage to the eeprom and set the addr */
/*****************************************************************************/
#ifdef use_ext_eeprom
void ee_send_start(unsigned int16 address, BOOLEAN read = TRUE)
{
   unsigned int8 romAddr = (unsigned int8)(address & 0x00FF);
   unsigned int8 slaveAddr = EE_Slave_Addr;
   #ifdef writeProtectPin
   output_low(writeProtectPin); // for write operations, clr WP pin
   #endif
   if (address > 0xFF) // if address > 8 bits
   { //add upper address bits (LSB of slave reserved are R/!W) 
      slaveAddr |= (unsigned int8)((address >> 7) & 0x000E) ;   
   }
   i2c_start(EEPROM);   // send start pulse on bus
   delay_ms(1);
   i2c_write(EEPROM, slaveAddr); // send slave address with R/!W clr (WRITE)
   i2c_write(EEPROM, romAddr); // rom address
   if (read) 
   {
      slaveAddr |= 0x01; // set the read bit
      delay_ms(1);
      i2c_start(EEPROM);
      delay_ms(1);
      i2c_write (EEPROM, slaveAddr); // send slave with R!W set (READ)
      #ifdef writeProtectPin
      output_high(writeProtectPin); // for read operations, set WP pin
      #endif
   }
}
#endif
/*****************************************************************************/
/* SAVE REGISTER TO EEPROM                                                   */
/* note all registers are saved at a page boundary for simplicity            */
/*****************************************************************************/
void ee_save_reg(unsigned int8 *reg, unsigned int16 eeLocation, unsigned int8 length)
#ifdef use_ext_eeprom
{
   int pageCnt = length/pageSize;
   int lastPageLength = length%pageSize;
   
   int i = 0; // number of pages to write
   int j = 0; // number of characters to write on last page
   
   for ( i = 0; i < pageCnt ; i++)  // pages to writes
   {
      ee_send_start(eeLocation, FALSE); // start the right
      for (j = 0; j < pageSize ; j++) // write 16 bytes (one page)
      {
         i2c_write(EEPROM, *(reg + j + (i * pageSize)));
         delay_ms(1);
      }
      i2c_stop();
      delay_ms(5);
      eeLocation += pageSize;
   }   
   if (lastPageLength > 0)
   {
      ee_send_start(eeLocation, FALSE);
      for (j = 0; j < lastPageLength; j++)
      {
         i2c_write(EEPROM, *(reg + j + (i * pageSize)));
         delay_ms(1);
      }
   }   
   i2c_stop(EEPROM);
   delay_ms(3);
   #ifdef writeProtectPin
   output_high(writeProtectPin); // for read operations, set WP pin
   #endif  
}
#else
{
   int i = 0;
   for (i = 0; i < length; i++)
   {
      write_eeprom(eeLocation+i, *(reg + i));
   }
}
#endif

/*****************************************************************************/
/* READ REGISTER FROM EEPROM                                                 */
/*****************************************************************************/
void ee_load_reg(unsigned int8 *reg, unsigned int16 eeLocation, unsigned int8 length)
#ifdef use_ext_eeprom
{
   ee_send_start(eeLocation, TRUE);
   for (int i = 0; i < length; i++)
   {
      unsigned int8 data = i2c_read(EEPROM,1); // acknoledge after each byte
      *(reg + i) = data;
   }
   i2c_read(EEPROM,0); // dummy read without acknowlege
   delay_ms(1);
   i2c_stop(EEPROM); // end page read, must preceed stop
   delay_ms(1);
}
#else
{
   int i = 0;
   for (i = 0; i < length; i++)
   {
      *(reg + i) = read_eeprom(eeLocation+i);
   }
}
#endif

/*****************************************************************************/
/* SAVE TO ALL PARAMS EEPROM                                                 */
/*****************************************************************************/
void params_save_to_ee()
{
   ee_save_reg(&PID, pid_params_ee_addr, pid_params_length);
   delay_ms(1);
   ee_save_reg(&chMap, channel_map_ee_addr, channel_map_length);
   delay_ms(1);
   ee_save_reg(&chMode, channel_mode_ee_addr, channel_mode_length);
   delay_ms(1);
   ee_save_reg(&adcCal, adc_cal_params_ee_addr, adc_cal_params_length);
   delay_ms(1);
   ee_save_reg(&sensorCal, sensor_cal_params_ee_addr, sensor_cal_params_length);
   delay_ms(1);
   ee_save_reg(&monCal, mon_cal_params_ee_addr, mon_cal_params_length);
   delay_ms(1);
   ee_save_reg(&paramsValid, params_valid_ee_addr, params_valid_length);
   delay_ms(1);
}
/*****************************************************************************/
/* LOAD ALL PARAMS FROM EEPROM                                               */
/*****************************************************************************/
void params_load_from_ee()
{
   ee_load_reg(&PID, pid_params_ee_addr, pid_params_length);
   ee_load_reg(&chMap, channel_map_ee_addr, channel_map_length);
   ee_load_reg(&chMode, channel_mode_ee_addr, channel_mode_length);
   ee_load_reg(&adcCal, adc_cal_params_ee_addr, adc_cal_params_length);
   ee_load_reg(&sensorCal, sensor_cal_params_ee_addr, sensor_cal_params_length);
   ee_load_reg(&monCal, mon_cal_params_ee_addr, mon_cal_params_length);
   ee_load_reg(&paramsValid, params_valid_ee_addr, params_valid_length);
}

/*****************************************************************************/
/* INITIALIZE PARAMS -- Include in main... Run on boot                       */
/*****************************************************************************/
void params_init()
{
   ee_load_reg(&paramsValid, params_valid_ee_addr, params_valid_length);
   delay_ms(1);
   if (paramsValid != isValid)
   {
      paramsValid = isValid;
      params_save_to_ee(); // saves the base parameters on first boot
   }
   else
   {
      params_load_from_ee();
   }
}

/*****************************************************************************/
/* INVALIDATE PARAMS -- Restarting will load defauts                         */
/*****************************************************************************/
void params_invalidate()
{
   paramsValid = invalidate;
   ee_save_reg(&paramsValid, params_valid_ee_addr, params_valid_length);
}

#endif
