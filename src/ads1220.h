#ifndef ADS1220
#define ADS1220

/*****************************************************************************/
/* SET UP PORTS FOR USE WITH DRIVER                                          */
/*****************************************************************************/
#define _CS0  PIN_D5 // chip select pins 
#define _CS1  PIN_D6
#define _CS2  PIN_D7
#define _CS3  PIN_B0

#pin_select SCK2=PIN_B1
#pin_select SDI2=PIN_B2
#pin_select SDO2=PIN_B3
#use spi(MASTER, SPI2, BAUD=1000000, MODE=1, STREAM=SPI_mon)
//!setup_spi2(SPI_MASTER | SPI_SCK_IDLE_LOW | SPI_XMIT_H_TO_L | SPI_CLK_DIV_64);

/*****************************************************************************/

/*****************************************************************************/
/* REGISTER DEFINITIONS                                                      */
/*****************************************************************************/
typedef enum {reg0 = 0x00,reg1 = 0x04 ,reg2 = 0x08, reg3 = 0x0C}adsReg;

//REGISTER 0 OPTIONS

#define IPp0n1 0x00
#define IPp0n2 0x10
#define IPp0n3 0x20
#define IPp1n2 0x30
#define IPp1n3 0x40
#define IPp2n3 0x50
#define IPp1n0 0x60
#define IPp3n2 0x70
#define IPp0nVss 0x80
#define IPp1nVss 0x90
#define IPp2nVss 0xA0
#define IPp3nVss 0xB0
#define IPVrpVrn 0xC0
#define IPVddVss 0xD0
#define IPhalfRail 0xE0

#define g1 0x00
#define g2 0x02
#define g4 0x04
#define g8 0x06
#define g16 0x08
#define g32 0x0A
#define g64 0x0C
#define g128 0x0E

#define PGAenabled 0x00
#define PGAbypass 0x01

//REGISTER 1 OPTIONS

#define DRn20 0x00
#define DRn45 0x20
#define DRn90 0x40
#define DRn175 0x60
#define DRn330 0x80
#define DRn600 0xA0
#define DRn1000 0xC0

#define MDnormal 0x00
#define MDduty 0x80
#define MDturbo 0x10

#define CMsingle 0x00
#define CMcont 0x40

#define TSdisable 0x00
#define TSenable 0x20

#define BCSoff 0x00
#define BCSon 0x01

//REGISTER 2 OPTIONS

#define REFinternal 0x00
#define REFp0n0 0x40
#define REFp1n1 0x80
#define REFVddVss 0xC0

#define FIRoff 0x00
#define FIR5060 0x10
#define FIR50 0x20
#define FIR60 0x30

#define PSWopen 0x00
#define PSWstart 0x08

#define Ioff 0x00
#define Iu10 0x01
#define Iu50 0x02
#define Iu100 0x03
#define Iu250 0x04
#define Iu500 0x05
#define Iu1000 0x06
#define Iu1500 0x07  

//REGISTER 3 OPTIONS

#define I1disabled 0x00
#define I1a0p1 0x20
#define I1a1 0x40
#define I1a2 0x60
#define I1a3n1 0x80
#define I1p0 0xA0
#define I1n0 0xC0

#define I2disabled 0x00
#define I2a0p1 0x04
#define I2a1 0x08
#define I2a2 0x0A
#define I2a3n1 0x10
#define I2p0 0x14
#define I2n0 0x18

#define drdyPin 0x00
#define doutPin 0x20

// REGISTER CONGIGURATIONS FOR THIS APP
#define reg0config IPp1n2|g16|PGAenabled // 0x38//
#define reg1config DRn20|MDnormal|CMsingle|TSDisable|BCSoff //0x00
#define reg2config REFinternal|FIR60|PSWopen|Ioff //0x30
#define reg3config I1disabled|I2disabled|drdyPin //0x00

//COMMANDS
#define ADSreset 0x06
#define ADSstart 0x08
#define ADSpowerDn 0x02
#define ADSreadData 0x10
#define ADSreadReg 0x20
#define ADSwriteReg 0x40

/*****************************************************************************/
/*****************************************************************************/
/* FUNCTIONS START HERE                                                      */
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* DESELECT ALL ADCs                                                         */
/*****************************************************************************/
void ads_deselect_all()
{   
   output_high(_CS0); // deselect all chip select pins
   output_high(_CS1);
   output_high(_CS2);
   output_high(_CS3); 
}

/*****************************************************************************/
/* SELECT DESIRED ADC                                                        */
/*****************************************************************************/
void ads_select_ch(int8 ch)
{
   ads_deselect_all();
   switch(ch)
   {
      case 0:
         output_low(_CS0);
      break; 
      case 1:
         output_low(_CS1);
      break;   
      case 2:
         output_low(_CS2);
      break;
      case 3:
         output_low(_CS3);
      break;              
   }
   delay_us(20);
}

/*****************************************************************************/
/* SELECT ADC BLOCK (2 ADCs/BLOCK)                                           */
/*****************************************************************************/
void ads_select_block(int8 block)
{
   ads_deselect_all();
   switch(block)
   {
      case 0:
         output_low(_CS0);
         output_low(_CS1);
      break; 
      case 1:         
         output_low(_CS2);
         output_low(_CS3);
      break;         
   }
   delay_us(20);
}

/*****************************************************************************/
/* WRITE THE COMMAND BYTE TO AN ADC                                          */
/*****************************************************************************/
void ads_write_command(int8 ch, unsigned int8 command)
{
   ads_select_ch(ch);
   spi_write2(command);
}

/*****************************************************************************/
/* WRITE THE COMMAND BYTE TO A BLOCK OF ADCs                                 */
/*****************************************************************************/
void ads_write_command_block(int8 block, unsigned int8 command)
{
   ads_select_block(block);
   spi_write2(command);
}

/*****************************************************************************/
/* PREPARE TO READ FROM AN ADC                                               */
/*****************************************************************************/
void ads_read_command(int8 ch, unsigned int8 command)
{
   ads_select_ch(ch);
   spi_read2(command);
}

/*****************************************************************************/
/* WRITE TO THE CONFIGURATION REGISTERS                                      */
/*****************************************************************************/
void ads_write_reg(int8 ch, adsReg regID, int8 data)
{
   unsigned int8 command = ADSwriteReg | regID | 0; // 0 is numbytes to write-1
   ads_write_command(ch, command);
   spi_write2(data);
   ads_deselect_all();
}

/*****************************************************************************/
/* READ FROM THE CONFIGURATION REGISTERS                                     */
/*****************************************************************************/
unsigned int8 ads_read_reg(int8 ch, adsReg regID)
{
   unsigned int8 command = ADSreadReg | regID | 0;
   ads_read_command(ch, command);
   unsigned int8 data = spi_read2(0);
   ads_deselect_all();
   return data;
}


/*****************************************************************************/
/* START BLOCK CONVERSION                                                    */
/*****************************************************************************/
void ads_start_conv_block(int8 block)
{
   ads_write_command_block(block, ADSstart);
   delay_us(20);
   ads_deselect_all();
}

/*****************************************************************************/
/* READ THE MOST RECENT CONVERSION                                           */
/*****************************************************************************/
signed int32 ads_read_data(int8 ch)
{
   union Data
   {
      unsigned int8 dBytes[4];
      signed int32 dWord;
   }data;
   
   ads_read_command(ch, ADSreadData);
   data.dBytes[3] = 0;
   data.dBytes[2] = spi_read2(0);
   data.dBytes[1] = spi_read2(0);
   data.dBytes[0] = spi_read2(0);
   
   ads_write_command(ch, ADSstart);
   ads_deselect_all();
   return data.dWord;
}

/*****************************************************************************/
/* INITIALIZE A CHANNEL                                                      */
/*****************************************************************************/

void ADS1220init(int8 ch, rc0=reg0config, rc1=reg1config, rc2=reg2config, rc3=reg3config)
{
   ads_write_command(ch, ADSreset);   //reset the device
   delay_us(300);                    
   ads_write_reg(ch, reg0, rc0);   //send default configurations
   ads_write_reg(ch, reg1, rc1);
   ads_write_reg(ch, reg2, rc2);
   ads_write_reg(ch, reg3, rc3);  
}
#endif 
