#ifndef cmdHandlerC
#define cmdHandlerC
#include <commandHandler.h>

/* LIST OF DIAGNOSTIC COMMAND KEYWORDS AND THEIR FUNCTIONS */
struct command cmd_list[] = {
   {"gr",       &getRev},
   {"gs",       &getSN},
   {"gChMap",   &getOPchMap},
   {"sChMap",   &setOPchMap},
   {"gChMode",  &getIPchMode},
   {"sChMode",  &setIPchMode},
   {"gPID",     &getPIDvals},
   {"sPID",     &setPIDvals},
//!   {"enaPID",   &enablePID},
//!   {"disPID",   &disablePID},
   {"gSP",      &getSetPoint},
   {"sSP",      &setSetPoint},
   {"gSCals",   &getAllSensorCalParams},
   {"gSCal",    &getSensorCalParam},
   {"sSCal",    &setSensorCalParam},
   {"gMCal",    &getMonitorCalParam},
   {"sMCal",    &setMonitorCalParam},
   {"gMon",     &getMonitorValue},
   {"gPIDdata", &getPIDdata},
   {"gIPdata",  &getIPdata},
   {"gManOP",   &getManOPvals},
   {"sManOP",   &setManOPvals},
   {"\0", &invalidCmd}
};

/* LIST OF RESPONSE MESSAGES */
enum resp_e { 
   SUCCESS,
   INV_CMD,
   INV_PARAM 
};

resp resp_list[] = {
   {SUCCESS, "SUCCESS"},
   {INV_CMD, "INVALID COMMAND"},
   {INV_PARAM, "INVALID PARAMETER"}
};

/*****************************************************************************/
/* DIAGNOSTIC COMMAND DEFINITIONS                                            */
/*****************************************************************************/
                          
int8 getRev(unsigned int8 rec){
   sprintf(retData+strlen(retData), "%s,", version);
   return SUCCESS;
}

int8 getSN(unsigned int8 rec){
   sprintf(retData+strlen(retData), "%s,", serialID);
   return SUCCESS;
}

int8 getOPchMap(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8 arg1;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   /*** GET CHANNEL MAP ***************/ 
   if      (0 == chMap[arg1-1]) sprintf(retData+strlen(retData), "%d,%c,", arg1, 'X');
   else if (1 == chMap[arg1-1]) sprintf(retData+strlen(retData), "%d,%c,", arg1, 'Y');
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 setOPchMap(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8 arg1;
   char arg2;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (1 != strlen(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = SERcmd[rec].p[3][0];
   
   /*** SET CHANNEL MAP ***************/
   if      ('X' == arg2) chMap[arg1-1] = chX;
   else if ('Y' == arg2) chMap[arg1-1] = chY;
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 getIPchMode(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8 arg1;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   /*** GET INPUT MAP *****************/
   if      (0 == chMode[arg1-1]) sprintf(retData+strlen(retData), "%d,MANUAL,", arg1);
   else if (1 == chMode[arg1-1]) sprintf(retData+strlen(retData), "%d,MAGSNS,", arg1);
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 setIPchMode(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8 arg1;
   char *arg2;
   char *s_manual = "MANUAL";
   char *s_magsns = "MAGSNS";
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (0 == strlen(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = SERcmd[rec].p[3];
   
   /*** SET INPUT MAP *****************/
   if      (0 == strcmp(s_manual, arg2)) chMode[arg1-1] = MANUAL;
   else if (0 == strcmp(s_magsns, arg2)) chMode[arg1-1] = MAGSNS;
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 getPIDvals(unsigned int8 rec){
   /*** ARG CHECKS ********************/ 
   int8 arg1;
   char arg2;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (1 != strlen(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = SERcmd[rec].p[3][0];
   
   /*** GET P, I, D, or A(LL) VALUE **********/
   if      ('P' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].kP);
   else if ('I' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].kI);
   else if ('D' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].kD);
   else if ('A' == arg2) sprintf(retData+strlen(retData), "%d,%f,%f,%f,", arg1, PID[arg1-1].kP, PID[arg1-1].kI, PID[arg1-1].kD);
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 setPIDvals(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8  arg1;
   char  arg2;
   float arg3;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (1 != strlen(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = SERcmd[rec].p[3][0];
   
   if (!arg_is_float(SERcmd[rec].p[4])) return INV_PARAM;
   else arg3 = strtod(SERcmd[rec].p[4], '\0');
   
   /*** SET P, I, or D VALUE **********/
   if      ('P' == arg2) PID[arg1-1].kP = arg3;
   else if ('I' == arg2) PID[arg1-1].kI = arg3;
   else if ('D' == arg2) PID[arg1-1].kD = arg3;
   else return INV_PARAM;
   
   return SUCCESS;
}

//!int8 enablePID(unsigned int8 rec){
//!   /*** ARG CHECKS ********************/
//!   int8  arg1;
//!   
//!   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
//!   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
//!   
//!   PID[arg1-1].enable = TRUE;
//!   
//!   return SUCCESS;
//!}
//!
//!int8 disablePID(unsigned int8 rec){
//!   /*** ARG CHECKS ********************/
//!   int8  arg1;
//!   
//!   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
//!   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
//!   
//!   PID[arg1-1].enable = FALSE;
//!   
//!   return SUCCESS;
//!}

int8 getSetPoint(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8  arg1;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   /*** GET SETPOINT ******************/ 
   sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].SP);
   return SUCCESS;
}

int8 setSetPoint(unsigned int8 rec){
   /*** ARG CHECKS ********************/   
   int8  arg1;
   float arg2;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (!arg_is_float(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = strtod(SERcmd[rec].p[3], '\0');
   
   /*** SET SETPOINT ******************/ 
   PID[arg1-1].SP = arg2;
   
   return SUCCESS;
}

int8 getAllSensorCalParams(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8  arg1;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   /*** GET ALL SENSOR CAL PARAMS *****/
   sprintf(retData+strlen(retData), "%d,", arg1);
   sprintf(retData+strlen(retData), "%f,", sensorCal[arg1-1].c0);
   sprintf(retData+strlen(retData), "%f,", sensorCal[arg1-1].c1);
   sprintf(retData+strlen(retData), "%f,", sensorCal[arg1-1].c2);
   sprintf(retData+strlen(retData), "%f,", sensorCal[arg1-1].c3);
   sprintf(retData+strlen(retData), "%f,", sensorCal[arg1-1].c4);
   sprintf(retData+strlen(retData), "%f,", sensorCal[arg1-1].c5);
   
   return SUCCESS;
}

int8 getSensorCalParam(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8 arg1;
   char arg2;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (1 != strlen(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = SERcmd[rec].p[3][0];
   
   /*** GET SENSOR CAL PARAM **********/
   if      ('0' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, sensorCal[arg1-1].c0);
   else if ('1' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, sensorCal[arg1-1].c1);
   else if ('2' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, sensorCal[arg1-1].c2);
   else if ('3' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, sensorCal[arg1-1].c3);
   else if ('4' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, sensorCal[arg1-1].c4);
   else if ('5' == arg2) sprintf(retData+strlen(retData), "%d,%f,", arg1, sensorCal[arg1-1].c5);
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 setSensorCalParam(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8  arg1;
   char  arg2;
   float arg3;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (1 != strlen(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = SERcmd[rec].p[3][0];
   
   if (!arg_is_float(SERcmd[rec].p[4])) return INV_PARAM;
   else arg3 = strtod(SERcmd[rec].p[4], '\0');
   
   /*** SET SENSOR CAL PARAM **********/
   if      ('0' == arg2) sensorCal[arg1-1].c0 = arg3;
   else if ('1' == arg2) sensorCal[arg1-1].c1 = arg3;
   else if ('2' == arg2) sensorCal[arg1-1].c2 = arg3;
   else if ('3' == arg2) sensorCal[arg1-1].c3 = arg3;
   else if ('4' == arg2) sensorCal[arg1-1].c4 = arg3;
   else if ('5' == arg2) sensorCal[arg1-1].c5 = arg3;
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 getMonitorCalParam(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   char *arg1;
   char *s_N15 = "N15";
   char *s_200 = "200";
   char *s_5V6 = "5V6";
   char *s_5VA = "5VA";
   char *s_3V6X = "3V6X";
   char *s_3V3A = "3V3A";
   char *s_3V3D = "3V3D";
//!   char *s_all = "all";
   
   arg1 = SERcmd[rec].p[2];
   
   /*** GET MONITOR VALUES ************/
   if      (0 == strcmp(s_N15, arg1)) {
      sprintf(retData+strlen(retData), "%f,%f,", monCal.vN15[0],monCal.vN15[1]);
   }
   else if (0 == strcmp(s_200, arg1)) {
      sprintf(retData+strlen(retData), "%f,%f,", monCal.v200[0],monCal.v200[1]);
   }
   else if (0 == strcmp(s_5V6, arg1)) {
      sprintf(retData+strlen(retData), "%f,%f,", monCal.v5V6[0],monCal.v5V6[1]);
   }
   else if (0 == strcmp(s_5VA, arg1)) {
      sprintf(retData+strlen(retData), "%f,%f,", monCal.v5VA[0],monCal.v5VA[1]);
   }
   else if (0 == strcmp(s_3V6X, arg1)) {
      sprintf(retData+strlen(retData), "%f,%f,", monCal.v3V6X[0],monCal.v3V6X[1]);
   }
   else if (0 == strcmp(s_3V3A, arg1)) {
      sprintf(retData+strlen(retData), "%f,%f,", monCal.v3V3A[0],monCal.v3V3A[1]);
   }
   else if (0 == strcmp(s_3V3D, arg1)) {
      sprintf(retData+strlen(retData), "%f,%f,", monCal.v3V3D[0],monCal.v3V3D[1]);
   }
//!   else if (0 == strcmp(s_all, arg1)) {
//!      sprintf(retData+strlen(retData), "%f,%f,", monCal.vN15[0],monCal.vN15[1]);
//!      sprintf(retData+strlen(retData), "%f,%f,", monCal.v200[0],monCal.v200[1]);
//!      sprintf(retData+strlen(retData), "%f,%f,", monCal.v5V6[0],monCal.v5V6[1]);
//!      sprintf(retData+strlen(retData), "%f,%f,", monCal.v5VA[0],monCal.v5VA[1]);
//!      sprintf(retData+strlen(retData), "%f,%f,", monCal.v3V6X[0],monCal.v3V6X[1]);
//!      sprintf(retData+strlen(retData), "%f,%f,", monCal.v3V3A[0],monCal.v3V3A[1]);
//!      sprintf(retData+strlen(retData), "%f,%f,", monCal.v3V3D[0],monCal.v3V3D[1]);
//!   }
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 setMonitorCalParam(unsigned int8 rec){

   return SUCCESS;
}

int8 getMonitorValue(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   char *arg1;
   char *s_N15 = "N15";
   char *s_200 = "200";
   char *s_5V6 = "5V6";
   char *s_5VA = "5VA";
   char *s_3V6X = "3V6X";
   char *s_3V3A = "3V3A";
   char *s_3V3D = "3V3D";
   char *s_all = "all";
   
   arg1 = SERcmd[rec].p[2];
   
   /*** GET MONITOR VALUES ************/
   if      (0 == strcmp(s_N15, arg1)) {
      sprintf(retData+strlen(retData), "%.2f,", monitorVals.vN15);
   }
   else if (0 == strcmp(s_200, arg1)) {
      sprintf(retData+strlen(retData), "%.1f,", monitorVals.v200);
   }
   else if (0 == strcmp(s_5V6, arg1)) {
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v5V6);
   }
   else if (0 == strcmp(s_5VA, arg1)) {
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v5VA);
   }
   else if (0 == strcmp(s_3V6X, arg1)) {
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v3V6X);
   }
   else if (0 == strcmp(s_3V3A, arg1)) {
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v3V3A);
   }
   else if (0 == strcmp(s_3V3D, arg1)) {
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v3V3D);
   }
   else if (0 == strcmp(s_all, arg1)) {
      sprintf(retData+strlen(retData), "%.2f,", monitorVals.vN15);
      sprintf(retData+strlen(retData), "%.1f,", monitorVals.v200);
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v5V6);
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v5VA);
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v3V6X);
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v3V3A);
      sprintf(retData+strlen(retData), "%.3f,", monitorVals.v3V3D);
   }
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 getPIDdata(unsigned int8 rec){
  /*** ARG CHECKS ********************/ 
   int8 arg1;
   char *arg2;
   char *s_PV = "PV";
   char *s_CV = "CV";
   char *s_PVold = "PVold";
   char *s_I = "I";
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   arg2 = SERcmd[rec].p[3];
   
   /*** GET PV, CV, PVold, I, or A(LL) VALUE **********/
   if      (0 == strcmp(s_PV, arg2))     sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].PV);
   else if (0 == strcmp(s_CV, arg2))     sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].CV);
   else if (0 == strcmp(s_PVold, arg2))  sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].PVold);
   else if (0 == strcmp(s_I, arg2))      sprintf(retData+strlen(retData), "%d,%f,", arg1, PID[arg1-1].I);
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 getIPdata(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8  arg1;
   char *arg2;
   char *s_raw = "raw";
   char *s_calibrated = "calib";
   char *s_p0 = "p0";
   char *s_poles = "poles";
   char *s_real = "real";
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   arg2 = SERcmd[rec].p[3];
   
   /*** GET INPUT DATA ****************/
   if      (0 == strcmp(s_raw, arg2)) {
      sprintf(retData+strlen(retData), "%d,", arg1);
      sprintf(retData+strlen(retData), "%Ld,", (signed int32)adcVals[arg1-1].sinRaw);
      sprintf(retData+strlen(retData), "%Ld,", (signed int32)adcVals[arg1-1].cosRaw);
   }
   else if (0 == strcmp(s_calibrated, arg2)) {
      sprintf(retData+strlen(retData), "%d,", arg1);
      sprintf(retData+strlen(retData), "%Ld,", (signed int32)adcVals[arg1-1].sinCounts);
      sprintf(retData+strlen(retData), "%Ld,", (signed int32)adcVals[arg1-1].cosCounts);
   }
   else if (0 == strcmp(s_p0, arg2)) {
      sprintf(retData+strlen(retData), "%d,", arg1);
      sprintf(retData+strlen(retData), "%f,", adcVals[arg1-1].p0);
   }
   else if (0 == strcmp(s_poles, arg2)) {
      sprintf(retData+strlen(retData), "%d,", arg1);
      sprintf(retData+strlen(retData), "%d,", adcVals[arg1-1].npoles);
   }
   else if (0 == strcmp(s_real, arg2)) {
      sprintf(retData+strlen(retData), "%d,", arg1);
      sprintf(retData+strlen(retData), "%f,", adcVals[arg1-1].pReal);
   }
   else return INV_PARAM;
   
   return SUCCESS;
}

int8 getManOPvals(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8 arg1;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   /*** GET MANUAL OUTPUT VALUE ***************/ 
   sprintf(retData+strlen(retData), "%d,%f,", arg1, manualOutputValues[arg1-1]);
   
   return SUCCESS;
}

int8 setManOPvals(unsigned int8 rec){
   /*** ARG CHECKS ********************/
   int8 arg1;
   float arg2;
   
   if (!is_valid_channel(SERcmd[rec].p[2])) return INV_PARAM;
   else arg1 = strtoul(SERcmd[rec].p[2],'\0',10);
   
   if (!arg_is_float(SERcmd[rec].p[3])) return INV_PARAM;
   else arg2 = strtod(SERcmd[rec].p[3], '\0');
   
   /*** SET MANUAL OUTPUT VALUE ***************/
   if (arg2 > op_upper_bound) manualOutputValues[arg1-1] = op_upper_bound;
   else if (arg2 < op_lower_bound) manualOutputValues[arg1-1] = op_lower_bound;  
   else manualOutputValues[arg1-1] = arg2;
   
   return SUCCESS;
}

int8 invalidCmd(unsigned int8 rec){
   return INV_CMD;
}

/*****************************************************************************/
/* ARG VALIDATION HELPER FUNCS                                               */
/*****************************************************************************/
BOOLEAN arg_is_float(char *arg){
   BOOLEAN decimal = FALSE;
   for (int i=0; i<strlen(arg); i++){
      if (!isdigit(arg[i])){
         // Check for negative sign
         if ('-' != arg[i]){
            // Check for decimal
            if ('.' != arg[i]) return FALSE;
            else if  (decimal) return FALSE;
            else decimal = TRUE;
         }
         else if (0 != i) return FALSE;
         else if (1 == strlen(arg)) return FALSE;
      }
   }
   return TRUE;
}

BOOLEAN is_valid_channel(char *arg){
    if (1 != strlen(arg)) return FALSE;
    else if ('1' != arg[0] && '2' != arg[0]) return FALSE;
    return TRUE;
}

/*****************************************************************************/
/* ASCII HEX CHARACTER TO INT                                                */
/*****************************************************************************/
int8 hex_to_int(char ascii){
   if (ascii >= '0' && ascii <= '9') return ascii-'0';
   else if (ascii >= 'A' && ascii <= 'F') return ascii-'A'+10;
   else return 0xFF;
}

/*****************************************************************************/
/* COMMAND HANDLER/PARSER FUNCS                                              */
/*****************************************************************************/
void echo_cmd(unsigned int8 rec){
   fprintf(SERIAL, "%c", SERcmd[rec].t);
   while (SERcmd[rec].recReadIndex <= SERcmd[rec].recWriteIndex)
   {
      if (SERcmd[rec].recReadIndex != 0) fprintf(SERIAL, "%c", delimiter);
      fprintf(SERIAL, "%s", SERcmd[rec].p[SERcmd[rec].recReadIndex]);
      SERcmd[rec].recReadIndex++;
   }
   SERcmd[rec].recReadIndex=0;
}

int8 command_parser(unsigned int8 rec){
   char *cmd_adr = SERcmd[rec].p[0];
   char *cmd_str = SERcmd[rec].p[1];
//!   int8 devID = input_a();
   int8 devID = 0;
   int8 i = 0;
   
   if (hwID == cmd_adr[0]){
      if (hex_to_int(cmd_adr[1]) == (devID & 0x0F)){
         while (0 != strlen(cmd_list[i].cmd_name)){
            if (0 == strcmp(cmd_str, cmd_list[i].cmd_name))
            {
               int8 return_code = cmd_list[i].cmd_func(rec);
               return return_code;
            }
            i++;
         }
         return INV_CMD; // INVALID_CMD
      } else return 254; // NOT THIS ADDRESS
   } else return 255; // NOT THIS DEVICE TYPE
}

void command_handler_task(){
   while (getNextSERReadIndex())
   {
      int8 return_code = command_parser(SRI);
      //echo_cmd(SRI);
      sprintf(retData + strlen(retData), "%s", resp_list[return_code].msg);
      fprintf(SERIAL, "$%s,%s\n", SERcmd[SRI].p[0], retData);
      
      resetSERcmd(SRI);
   }
}

#endif
