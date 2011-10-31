/**
 * @file: serial.c
 * @brief: UART1 user input and output
 * @authors: Roy Feng, Ivan Grabovickic, Leon Ng, Abhishek Pandit
 * @date: 2010/05/19
 */
 
#include "../shared/rtx_inc.h"
#include "dbug.h"

/*
 * Global Variables
 */
volatile BYTE CharIn = '!';
volatile BOOLEAN Caught = TRUE;
volatile BYTE CharOut = '\0';
CHAR StringHack[] = "  was caught\n\r ";

/*
 * gcc expects this function to exist
 */
int __main( void )
{
    return 0;
}

/*
 * This function is called by the assembly ISR code
 */
VOID c_serial_handler( VOID )
{
    BYTE temp;

    //acknowledge the interrupt
	temp = SERIAL1_UCSR;    
    
    //see if data is waiting to be read   
    if( temp & 1 )
    {
        //read the data from the serial port
		CharIn = SERIAL1_RD;
        Caught = FALSE;
    }

    // See if port is ready to accept data    
    else if ( temp & 4 )
    {
       //check if enter was pressed
	   if( CharOut == 13 ){
           //output new line
		   SERIAL1_WD = '\n';
           while( !(SERIAL1_UCSR & 4) );
           SERIAL1_WD = '\r';
		   
		   //disable tx interupt
		   SERIAL1_IMR = 2;
       } else {
           //write data to serial port
		   SERIAL1_WD = CharOut;
           
		   //disable tx interupt
		   SERIAL1_IMR = 2;
       }
    }
    return;
}

SINT32 coldfire_vbr_init( VOID )
{
    /*
     * Move the VBR into real memory
     *
     * DG: actually, it'll already be here.
     */
    asm( "move.l %a0, -(%a7)" );
    asm( "move.l #0x10000000, %a0 " );
    asm( "movec.l %a0, %vbr" );
    asm( "move.l (%a7)+, %a0" );
    
    return RTX_SUCCESS;
}

/*
 * Entry point, check with m68k-coff-nm
 */
int main( void )
{
	UINT32 mask;

    //disable all interupts 
    asm( "move.w #0x2700,%sr" );
    
    coldfire_vbr_init();
        
    //store the serial ISR at user vector #64
    asm( "move.l #asm_serial_entry,%d0" );
    asm( "move.l %d0,0x10000100" );

    //reset the entire UART 
    SERIAL1_UCR = 0x10;

    //reset the receiver 
    SERIAL1_UCR = 0x20;
    
    //reset the transmitter 
    SERIAL1_UCR = 0x30;

    //reset the error condition
    SERIAL1_UCR = 0x40;

    //install the interupt
    SERIAL1_ICR = 0x17;
    SERIAL1_IVR = 64;

    //enable interrupts on rx only
    SERIAL1_IMR = 0x02;

    //set the baud rate
    SERIAL1_UBG1 = 0x00;
#ifdef _CFSERVER_           /* add -D_CFSERVER_ for cf-server build */
    SERIAL1_UBG2 = 0x49;    /* cf-server baud rate 19200 */ 
#else
    SERIAL1_UBG2 = 0x92;    /* lab board baud rate 9600 */
#endif 

    //set clock mode
    SERIAL1_UCSR = 0xDD;

    //setup the UART (no parity, 8 bits )
    SERIAL1_UMR = 0x13;
    
    //setup the rest of the UART (noecho, 1 stop bit )
    SERIAL1_UMR = 0x07;

    //setup for transmit and receive
    SERIAL1_UCR = 0x05;

    //enable interupts
    mask = SIM_IMR;
    mask &= 0x0003dfff;
    SIM_IMR = mask;

    
    //enable all interupts
    asm( "move.w #0x2000,%sr" );

    rtx_dbug_outs((CHAR *) "Type q on RTX terminal to quit.\n\r" );
    
    //busy Loop
    while( CharIn != 'q' )
    {
        if( !Caught )
        {
            if( CharIn == 'q' ) break;
            CharOut = CharIn;
            Caught = TRUE;
            
            //enable tx interrupts/
            SERIAL1_IMR = 3;

#ifdef _DEBUG_
            // Insert new char to output string
            StringHack[0] = CharOut == 13 ? 'r' : CharOut;
            rtx_dbug_outs( StringHack );
#endif
        }
    }

    //disable all interupts
    asm( "move.w #0x2700,%sr" );

    //reset globals so we can run again
    CharIn = '\0';
    Caught = TRUE;
    return 0;
}
