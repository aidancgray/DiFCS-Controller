#ifndef cmd_buffer
#define cmd_buffer

/*****************************************************************************/
/* DIFCS COMMAND BUFFER                                                      */
/* The beginning of a message is signified by an address record. The address */
/* is 4 characters. The first 2 are the hardware identifier (TC - temperature*/
/* controller, DC - difics controller, etc..) The second 2 characters dev-   */
/* ice ID... Valid values are 00 thru 0F. A device ID of FF writes to all    */
/* boards. This might be used to feed angles data to all listeners.          */
/*****************************************************************************/

// t,p0,p1,p2,p3,p4,p5,p6
// t is the start character, p0 is the address, p1 is command, p2-6 are params
// these characters used for storing messages

#define startChr '~' // start character. Also diagnostic command type
#define diagCmdType startChr
#define endChr '\n'
#define delimiter ','
#define ignoreChr1 '\r'
#define ignoreChr2 ' '
#define noRecordFound (char)0xFF

#define numRecords 4
#define numParam 7
#define lenParam 25

#define useSerial
//#define useTCPIP

typedef enum cmdMode {tcp, ser};

// this structure holds a command. This is used by the serial routine as well
// as the TCP routine to store an incoming command + parameters
// in the case of TCP socket id is also passed in this structure
typedef struct cmd 
{
   BOOLEAN full;        // if full, command is ready to execute
   unsigned int8 chrIndex;
   unsigned int8 paramIndex;
   unsigned int8 recReadIndex;
   unsigned int8 recWriteIndex;
   #ifdef useTCPIP
   TCP_SOCKET s;              // socket ID for TCP commands 
   #endif
   char t;
   char p[numParam][lenParam];   // only used with ~ type command
};

#ifdef useSerial
struct cmd SERcmd[numRecords]; // holds serial command 
unsigned int8 SRI = 0; //usb readindex
unsigned int8 SWI = 0; //usb write index
#endif

#ifdef useTCPIP
struct cmd TCPcmd[numRecords]; // holds TCP command
unsigned int8 TRI = 0; //usb readindex
unsigned int8 TWI = 0; //usb write index
#endif

char retData[100]; // this is used to hold return data 

#ifdef useSerial
/*****************************************************************************/
/* RESET SER COMMAND -- ready for a new command                              */
/*****************************************************************************/
void resetSERcmd(unsigned int8 recNum)
{
   if (recNum < numRecords)
   {
      for (int i = 0; i <numParam; i ++)
      {
         SERcmd[recNum].p[i][0] = '\0';
      }
      SERcmd[recNum].t = 0;
      SERcmd[recNum].chrIndex = 0;
      SERcmd[recNum].paramIndex = 0;
      SERcmd[recNum].recReadIndex = 0;
      SERcmd[recNum].recWriteIndex = 0;
      SERcmd[recNum].full = FALSE;
      retData[0] = '\0';
   }
}

/*****************************************************************************/
/* Get next serial read index (SRI)                                          */
/*****************************************************************************/
BOOLEAN getNextSERReadIndex()
{
   while (!SERcmd[SRI].full)
   {
      if (SRI == SWI) return FALSE;
      SRI +=1;
      if (SRI >= numRecords) SRI=0;
   }
   return TRUE;
}

/*****************************************************************************/
/* Get next serial write index (SWI)                                         */
/*****************************************************************************/
void setNextSERWriteIndex()
{
   SWI +=1;
   if (SWI >= numRecords) SWI=0;
}
#endif

#ifdef useTCPIP
/*****************************************************************************/
/* RESET TCP COMMAND -- ready for a new command                              */
/*****************************************************************************/
void resetTCPcmd(unsigned int8 recNum)
{
   if(recNum < numRecords)
   {
      for (int i = 0; i <numParam; i ++)
      {
         TCPcmd[recNum].p[i][0] = '\0';
      }
      TCPcmd[recNum].t = 0;
      TCPcmd[recNum].chrIndex = 0;
      TCPcmd[recNum].paramIndex = 0;
      TCPcmd[recNum].full = FALSE;
   }
}

/*****************************************************************************/
/* Get next TCP read index (TRI)                                             */
/*****************************************************************************/
BOOLEAN getNextTCPReadIndex()
{
   while (!TCPcmd[TRI].full)
   {
      if (TRI == TWI) return FALSE;
      TRI +=1;
      if (TRI >= numRecords) TRI=0;
   }
   return TRUE;
}

/*****************************************************************************/
/* Get next TCP write index (TWI)                                            */
/*****************************************************************************/
void setNextTCPWriteIndex()
{
   TWI +=1;
   if (TWI >= numRecords) TWI=0;
}
#endif
#endif

