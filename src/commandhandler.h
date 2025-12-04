#ifndef cmdHandler
#define cmdHandler
#include <parameters.h>

#define hwID 'D'   // DiFCS

/*****************************************************************************/
/* RESPONSE MESSAGES                                                         */
/*****************************************************************************/
typedef struct response {
   int8 id;
   char *msg;
} resp;

//!#define INVALID_DATA "error - invalid data"
//!#define invalidTime "error - invalid TO value"
//!#define overrun "error - buffer overrun"
//!#define dcVoltage "VDC:%f1"
//!#define USBVoltage "VUSB:%f"
//!#define USBupstream "VUSBup:%f"
//!#define plus5V "5V Rail:%f"
//!#define plus3V3 "3V3 Rai:%f"
//!#define allVoltages "All Voltages = %f, %f, %f, %f, %f"
//!#define USBdownstream "USB downstream status:\r\nPG:%s\r\nch0:%s\r\nch1:%s"

/*****************************************************************************/
/* DIAGNOSTIC MESSGAGE DUMP                                                  */
/*****************************************************************************/
//!#define voltageMsg "V0:%f\r\nV1:%f\r\nV2:%f\r\nV3:%f\r\nV4:%f"
//!#define TimerMsg "T0:%Lu\r\nT1:%Lu\r\nT2:%Lu\r\Tint:%Lu\r\nTtick:%Lu"
//!#define ioStatusMsg "Inputs:%u\r\nOutputs:%u\r\nStatus:%u"
//!#define diagnosticMsg "V0:%f\r\nV1:%f\r\nV2:%f\r\nV3:%f\r\nV4:%f\r\nT0:%Lu\r\nT1:%Lu\r\nT2:%Lu\r\nTint:%Lu\r\nTtick:%Lu\r\nInputs:%u\r\nOutputs:%u\r\nStatus:%u"

/*****************************************************************************/
/* DIAGNOSTIC COMMAND PROTOTYPES                                             */
/*****************************************************************************/
int8 getRev(unsigned int8);                // gr 
int8 getSN(unsigned int8);                 // gs

int8 getOPchMap(unsigned int8);            // gChMap,   {1|2}
int8 setOPchMap(unsigned int8);            // sChMap,   {1|2}, {X|Y}

int8 getIPchMode(unsigned int8);           // gChMode,  {1|2}
int8 setIPchMode(unsigned int8);           // sChMode,  {1|2}, {MANUAL|MAGSNS}
   
int8 getPIDvals(unsigned int8);            // gPID,     {1|2}, {P|I|D}
int8 setPIDvals(unsigned int8);            // sPID,     {1|2}, {P|I|D}, <float>

int8 enablePID(unsigned int8);             // enaPID,   {1|2}
int8 disablePID(unsigned int8);            // disPID,   {1|2}

int8 getSetPoint(unsigned int8);           // gSP,      {1|2}
int8 setSetPoint(unsigned int8);           // sSP,      {1|2}, <float>

int8 getAllSensorCalParams(unsigned int8); // gSCals,   {1|2}
int8 getSensorCalParam(unsigned int8);     // gSCal,    {1|2}, {0|1|2|3|4|5}
int8 setSensorCalParam(unsigned int8);     // sSCal,    {1|2}, {0|1|2|3|4|5}, <float>

int8 getMonitorCalParam(unsigned int8);    // gMCal,    {N15|200|5V6|5VA|3V6X|3V3A|3V3D}
int8 setMonitorCalParam(unsigned int8);    // sMCal,    {N15|200|5V6|5VA|3V6X|3V3A|3V3D}, <float>, <float>
int8 getMonitorValue(unsigned int8);       // gMon,     {N15|200|5V6|5VA|3V6X|3V3A|3V3D|all}

int8 getPIDdata(unsigned int8);            // gPIDdata, {1|2}, {PV|CV|PVold|I}

int8 getIPdata(unsigned int8);             // gIPdata,  {1|2}, {raw|calib|p0|poles|real}
   
int8 getManOPvals(unsigned int8);          // gManOP,   {1|2}  
int8 setManOPvals(unsigned int8);          // sManOP,   {1|2}, <float>  

int8 setFilterOn(unsigned int8);           // sFiltOn
int8 setFilterOff(unsigned int8);          // sFiltOff

int8 invalidCmd(unsigned int8);

struct command {
   char *cmd_name;
   int8 (*cmd_func)(unsigned int8);
};

BOOLEAN arg_is_float(char *);
BOOLEAN is_valid_channel(char *);
int8 hex_to_int(char);
void echo_cmd(unsigned int8 rec);
int8 command_parser(unsigned int8 rec);
void command_handler_task();

#endif
