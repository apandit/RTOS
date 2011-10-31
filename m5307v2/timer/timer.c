/**
 * @file: timer.c
 * @brief: timer 0 smaple code
 * @author: ECE354 Lab Instructors and TAs
 * @author: Irene Huang
 * @date: 2010/05/03
 */

#include "../shared/rtx_inc.h"
#include "dbug.h"

/*
 * Global Variables
 */
SINT32 Counter = 0;


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
VOID c_timer_handler( VOID )
{
    Counter++;
    rtx_dbug_out_char('.');
    /*
     * Ack the interupt
     */
    TIMER0_TER = 2;
}




SINT32 coldfire_vbr_init( VOID )
{
    /*
     * Move the VBR into real memory
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
     * Store the timer ISR at auto-vector #6
     */
    asm( "move.l #asm_timer_entry,%d0" );
    asm( "move.l %d0,0x10000078" );

    /*
     * Setup to use auto-vectored interupt level 6, priority 3
     */
    TIMER0_ICR = 0x9B;

    /*
     * Set the reference counts, ~10ms
     */
    TIMER0_TRR = 1758;

    /*
     * Setup the timer prescaler and stuff
     */
    TIMER0_TMR = 0xFF1B;

    /*
     * Set the interupt mask
     */
    mask = SIM_IMR;
    mask &= 0x0003fdff;
    SIM_IMR = mask;    

    /* Let the timer interrupt fire, lower running priority */
    asm( "move.w #0x2000,%sr" );
    
    /* Wait for 5 seconds to pass */
    rtx_dbug_outs( (CHAR *) "Waiting approx. 5 seconds for Counter > 500\n\r" );
    Counter=0;
    while( Counter < 500 );
    rtx_dbug_outs( (CHAR *) "Counter >= 500\n\r" );
    return 0;
}
