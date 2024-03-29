/**
 * @file: serial.c
 * @brief: UART1/second serial port/rtx terminal I/O sample code
 * @author: ECE Lab Instructors and TAs
 * @author: Irene Huang
 * @date: 2010/05/03
 */
#include "../shared/rtx_inc.h"
#include "dbug.h"

/*
 * Global Variables
 */
volatile BYTE CharIn = '!';
volatile BOOLEAN Caught = TRUE;
volatile BYTE CharOut = '\0';
CHAR StringHack[] = "You Typed a Q\n\r";



/*
 * gcc expects this function to exist
 */
int __main( void )
{
    return 0;
}


/*
 * This function is called by the assembly STUB function
 */
VOID c_serial_handler( VOID )
{
    BYTE temp;

    temp = SERIAL1_UCSR;    /* Ack the interrupt */

#ifdef _DEBUG_
    rtx_dbug_outs((CHAR *) "Enter: c_serial_handler ");
#endif /* _DEBUG */
    
    /* See if data is waiting.... */    
    if( temp & 1 )
    {
#ifdef _DEBUG_
        rtx_dbug_outs((CHAR *) "reading data: ");
        rtx_dbug_out_char(CharIn);
    	rtx_dbug_outs((CHAR *) "\r\n");
#endif /* _DEBUG */
        CharIn = SERIAL1_RD;
        Caught = FALSE;
    }

    /* See if port is ready to accept data */    
    else if ( temp & 4 )
    {
#ifdef _DEBUG_
        rtx_dbug_outs((CHAR *) "writing data: ");
        rtx_dbug_out_char(CharOut);
    	rtx_dbug_outs((CHAR *) "\r\n");
#endif /* _DEBUG_*/
        SERIAL1_WD = CharOut;   /* Write data to port */
        SERIAL1_IMR = 2;        /* Disable tx Interupt */
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

    /* Disable all interupts */
    asm( "move.w #0x2700,%sr" );
    
    coldfire_vbr_init();
    
    /*
     * Store the serial ISR at user vector #64
     */
    asm( "move.l #asm_serial_entry,%d0" );
    asm( "move.l %d0,0x10000100" );

    /* Reset the entire UART */
    SERIAL1_UCR = 0x10;

    /* Reset the receiver */
    SERIAL1_UCR = 0x20;
    
    /* Reset the transmitter */
    SERIAL1_UCR = 0x30;

    /* Reset the error condition */
    SERIAL1_UCR = 0x40;

    /* Install the interupt */
    SERIAL1_ICR = 0x17;
    SERIAL1_IVR = 64;

    /* enable interrupts on rx only */
    SERIAL1_IMR = 0x02;

    /* Set the baud rate */
    SERIAL1_UBG1 = 0x00;
#ifdef _CFSERVER_           /* add -D_CFSERVER_ for cf-server build */
    SERIAL1_UBG2 = 0x49;    /* cf-server baud rate 19200 */ 
#else
    SERIAL1_UBG2 = 0x92;    /* lab board baud rate 9600 */
#endif /* _CFSERVER_ */

    /* Set clock mode */
    SERIAL1_UCSR = 0xDD;

    /* Setup the UART (no parity, 8 bits ) */
    SERIAL1_UMR = 0x13;
    
    /* Setup the rest of the UART (noecho, 1 stop bit ) */
    SERIAL1_UMR = 0x07;

    /* Setup for transmit and receive */
    SERIAL1_UCR = 0x05;

    /* Enable interupts */
    mask = SIM_IMR;
    mask &= 0x0003dfff;
    SIM_IMR = mask;

    
    /* Enable all interupts */
    asm( "move.w #0x2000,%sr" );

    rtx_dbug_outs((CHAR *) "Type Q or q on RTX terminal to quit.\n\r" );
    
    /* Busy Loop */
    while( CharIn != 'q' && CharIn != 'Q' )
    {
        if( !Caught )
        {
            Caught = TRUE;
            CharOut = CharIn;
            
            /* Nasty hack to get a dynamic string format, 
             * grab the character before turning the interrupts back on. 
             */
            StringHack[12] = CharIn;

            /* enable tx interrupts  */
            SERIAL1_IMR = 3;

	        /* Now print the string to debug, 
             * note that interrupts are now back on. 
             */
            rtx_dbug_outs( StringHack );
        }
    }

    /* Disable all interupts */
    asm( "move.w #0x2700,%sr" );

    /* Reset globals so we can run again */
    CharIn = '\0';
    Caught = TRUE;
    return 0;
}
