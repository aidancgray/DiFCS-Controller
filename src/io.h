#ifndef IO
#define IO

#use FIXED_IO( A_outputs=PIN_A5,PIN_A4 )
#use FIXED_IO( B_outputs=PIN_B3,PIN_B1,PIN_B0 )
#use FIXED_IO( C_outputs=PIN_C6,PIN_C5,PIN_C3,PIN_C2 )
#use FIXED_IO( D_outputs=PIN_D7,PIN_D6,PIN_D5 )
#use FIXED_IO( E_outputs=PIN_E2,PIN_E1,PIN_E0 )

#define A0           PIN_A0
#define A1           PIN_A1
#define A2           PIN_A2
#define A3           PIN_A3
#define _CS_ACC1     PIN_A4
#define _CS_ACC0     PIN_A5
#define MON_N15V     PIN_A6

#define _CS_YCOS     PIN_B0
#define SCLK_B       PIN_B1
#define MISO_B       PIN_B2
#define MOSI_B       PIN_B3
#define _LE          PIN_B4
#define CLR          PIN_B5

#define MON_200V     PIN_C0
#define MON_3V6X     PIN_C1
#define MOSI_A       PIN_C2
#define SCLK_A       PIN_C3
#define MISO_A       PIN_C4
#define TX_ENABLE    PIN_C5
#define TX_PIN       PIN_C6
#define RX_PIN       PIN_C7

#define MON_5V6      PIN_D0
#define MON_5VA      PIN_D1
#define MON_3V3D     PIN_D2
#define MON_3V3A     PIN_D3
#define RS485_FLT    PIN_D4
#define _CS_XSIN     PIN_D5
#define _CS_XCOS     PIN_D6
#define _CS_YSIN     PIN_D7

#define EN_EXC       PIN_E0
#define _SYNC_Y      PIN_E1
#define _SYNC_X      PIN_E2

#pin_select U1TX = PIN_C6
#pin_select U1RX = PIN_C7

// initialization states
#define portA_reset 0x00
#define portB_reset 0x00
#define portC_reset 0x00
#define portD_reset 0x00
#define portE_reset 0x00

#define portA_pullups 0xFF
#define portB_pullups 0x00
#define portC_pullups 0x00
#define portD_pullups 0xFF
#define portE_pullups 0x00

/*****************************************************************************/
/* INITIALIZE IO                                                             */
/*****************************************************************************/
void IO_init()
{
   output_a(portA_reset);
   output_b(portB_reset);
   output_c(portC_reset);
   output_d(portD_reset);
   output_e(portE_reset);
   
   port_a_pullups(portA_pullups);
   port_b_pullups(portB_pullups);
   port_c_pullups(portC_pullups);
   port_d_pullups(portD_pullups);
   port_e_pullups(portE_pullups);
   
   set_analog_pins(MON_N15V, MON_200V, MON_5V6, MON_5VA, MON_3V6X, MON_3V3A, MON_3V3D);
}

#endif
