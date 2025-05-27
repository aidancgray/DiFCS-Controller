#ifndef serialPort
#define serialPort
#include <cmdBuffer.h>
#include <io.h>

#define use_RS485 // adds RX enable pin//

#define oneByteCmdTestValue 128
#define UART_BUFFER_SIZE 60
#define UART_EOT_CHAR1 0x0D //Carriage Return
#define UART_EOT_CHAR2 0x0A //Line Feed
#define UART_SOT_CHAR  '~' // START CHARACTER
#define line_turnaround_char 0xFF

#bit UART_TXBuffer_Empty = 0x0FAC.1

char UART_BUFFER[UART_BUFFER_SIZE] = {0};
unsigned int8 UART_WR_PTR = 0;
unsigned int8 UART_RD_PTR = 0;
boolean BYTES_AVAILABLE= FALSE;

#ifdef use_RS485
#use rs232(baud=baudRate, xmit=TX_PIN, rcv=RX_PIN, enable=TX_ENABLE, stream=SERIAL, ERRORS)
#else
#use rs232(baud=baudRate, xmit=TX_PIN, rcv=RX_PIN, stream=SERIAL)
#endif

// Message format: [SOT]addr ,cmd, arg1, arg2, arg3[EOT]} 
// arguments are optional
/*****************************************************************************/
/*****************************************************************************/
/* FUNCTIONS START HERE                                                      */
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* SERIAL PORT ISR                                                           */
/*****************************************************************************/
#INT_RDA
void RX_isr()
{
   while (kbhit())
   {
      UART_BUFFER[UART_WR_PTR]=getch();
      UART_WR_PTR +=1;
      if (UART_WR_PTR>=UART_BUFFER_SIZE) UART_WR_PTR=0;
      BYTES_AVAILABLE=TRUE;
   }
}

/*****************************************************************************/
/* Get character from buffer                                                 */
/*****************************************************************************/
char getchFromBuffer()
{
   char data = UART_BUFFER[UART_RD_PTR];
   UART_RD_PTR +=1;
   if (UART_RD_PTR>=UART_BUFFER_SIZE) UART_RD_PTR=0;
   if (UART_RD_PTR == UART_WR_PTR) BYTES_AVAILABLE=FALSE;
   return data;
}

/*****************************************************************************/
/* Serial Task transfers bytes to command buffer                             */
/*****************************************************************************/
void serial_task()
{
   while(BYTES_AVAILABLE && SERcmd[SWI].full==FALSE)
   {  
      unsigned int8 rxChar = getchFromBuffer();
      if (rxChar == ignoreChr1 || rxChar == ignoreChr2)
      {
      }
      else if (rxChar == UART_SOT_CHAR)
      {
         resetSERcmd(SWI);
         SERcmd[SWI].t = rxChar;
      }
      else if (rxChar >= oneByteCmdTestValue)
      {
         resetSERcmd(SWI);
         SERcmd[SWI].t = rxChar;
         SERcmd[SWI].full = TRUE;
         setNextSERWriteIndex();
      }
      else if (rxChar == UART_EOT_CHAR2)
      {
         SERcmd[SWI].full = TRUE;
         SERcmd[SWI].recWriteIndex = SERcmd[SWI].paramIndex;
         SERcmd[SWI].chrIndex = 0;
         SERcmd[SWI].paramIndex = 0;
         setNextSERWriteIndex();
      }
      else if (rxChar == delimiter)
      {
         if (SERcmd[SWI].paramIndex < numParam-1)
         {
            SERcmd[SWI].paramIndex +=1;
            SERcmd[SWI].chrIndex = 0;
         }
         else
         {
            resetSERcmd(SWI); // exceed number of params... reset capture
            fprintf(SERIAL, retData);
         }
      }
      
      else
      {
         if (SERcmd[SWI].chrIndex < lenParam)
         {
            SERcmd[SWI].p[SERcmd[SWI].paramIndex][SERcmd[SWI].chrIndex] = rxChar;
            SERcmd[SWI].chrIndex += 1;
            SERcmd[SWI].p[SERcmd[SWI].paramIndex][SERcmd[SWI].chrIndex] = '\0';
         }
         else
         {
            resetSERcmd(SWI); // exceed number of characters... reset capture
            fprintf(SERIAL, retData);
         }
      }
   }          
}
           
/*****************************************************************************/
/* serialInit ROUTINE  - Sets up serial port                                 */
/*****************************************************************************/ 
void serial_init()
{
   for (int i = 0; i < numRecords; i++) 
   {
      resetSERcmd(i);
   }
   enable_interrupts(INT_RDA);
}


#endif
