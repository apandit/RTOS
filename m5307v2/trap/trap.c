/**
 * @file: trap.c
 * @brief: software interuupt (SWI) sample code
 * @author: ECE354 Lab Instructors and TAs
 * @author: Irene Huang
 * @date: 2010/05/30
 */

#include "../shared/rtx_inc.h"
#include "dbug.h"


/*
 * This function is called by the assembly STUB function
 */
VOID c_trap_handler( VOID )
{
    rtx_dbug_outs( (CHAR *) "Trap Handler!!\n\r" );
}


/*
 * gcc expects this function to exist
 */
int __main( void )
{
    return 0;
}


/*
 * Entry point, check with m68k-coff-nm
 */
int main( void )
{
    /* Load the vector table for TRAP #0 with our assembly stub
       address */
    asm( "move.l #asm_trap_entry,%d0" );
    asm( "move.l %d0,0x10000080" );

    /* Trap out three times */
    asm( "TRAP #0" );
    asm( "TRAP #0" );
    asm( "TRAP #0" );    
    return 0;
}
