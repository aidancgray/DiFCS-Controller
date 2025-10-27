#include <18F46Q10.h>

#device ADC=10

//!#FUSES WDT                      //Watch Dog Timer
//!#FUSES WDT4096                  //Watch Dog Timer uses 1:4096 Postscale

#use delay(internal=64000000)
#define baudRate 128000
#define numChannels 2

#define version  "20250812" // SW Version
#define serialID "0000"     // Unique Serial ID
#define magPP 1             // Magnetoresistive Sensor Pole Pitch in mm   
#define gainLvl 16          // 1, 2, 4, 8, 16, 32, 64, 128

