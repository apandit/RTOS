/**
 * @file: dbug.c
 * @brief: output to janusROM terminal by using trap #15
 * @author: ECE354 Lab Instructors and TAs
 * @author: Irene Huang
 * @date: 2010/05/03
 */

#include "dbug.h"

CHAR output[30];

/**
 * @brief: C Function wrapper for TRAP #15 function to output a character
 * @param: c the charcter to output to janusROM  
 */

VOID rtx_dbug_out_char( CHAR c )
{
#ifdef _DEBUG_
    /* Store registers */
    asm( "move.l %d0, -(%a7)" );
    asm( "move.l %d1, -(%a7)" );

    /* Load CHAR c into d1 */
    asm( "move.l 8(%a6), %d1" );  /* Standard Motorola syntax */ 
    //asm( "move.l (8, %a6), %d1" );/* Standard Motorola syntax */
    //asm( "move.l %a6@(8), %d1" ); /* Motorola 680x0 syntax developed by MIT */

    /*
    asm("move.l %0, %%d1"
       : // no output  
       :"g"(c)
       :"d1" 
       );
   */


    /* Setup trap function */
    asm( "move.l #0x13, %d0" );
    asm( "trap #15" );

    /* Restore registers  */
    asm(" move.l (%a7)+, %d1" );
    asm(" move.l (%a7)+, %d0" );
#endif
}


/**
 * @brief: Prints a C-style null terminated string
 * @param: s the string to output to janusROM terminal 
 */
SINT32 rtx_dbug_outs( CHAR* s )
{
#ifdef _DEBUG_
    if ( s == NULL )
    {
        return RTX_ERROR;
    }
    while ( *s != '\0' )
    {
        rtx_dbug_out_char( *s++ );
    }
#endif
    return RTX_SUCCESS;
}


// Convert number to string (char array)
// Writes to timerOutput char array
SINT32 WriteNumber( int number ){
	
	if( number < 0 ){
		number = number * -1;
	}

    int md = 0;
    int ctr = 0;

    if( number == 0 ){
        rtx_dbug_out_char( '0' );
        return RTX_SUCCESS;
    }

    while( number ){
        // Take new least significant digit
        md = number % 10;
		
		// Convert to char and place in array
        output[ctr] = md + '0';
       
        // Remove handled digit
        number /= 10;

        ctr++;
    }

	// Last increment of ctr is above last index
    ctr--;

	// Output in reverse order
    while( ctr >= 0 ){
        rtx_dbug_out_char( output[ctr] );
        ctr--;
    }

    return RTX_SUCCESS;
}

// Convert number to string (char array)
// Writes to timerOutput char array
SINT32 WriteHex( int number ){
    int md = 0;
    int ctr = 0;
	
	if( number < 0 ){
		number = number * -1;
	}
	
    if( number == 0 ){
        rtx_dbug_out_char( '0' );
        return RTX_SUCCESS;
    }

    while( number ){
        // Take new least significant digit
        md = number % 16;
		
		// Convert to char and place in array
        if( md > 9 ){
            output[ctr] = (md-10) + 'a';
        } else {
            output[ctr] = md + '0';
        }
       
        // Remove handled digit
        number /= 16;

        ctr++;
    }

	// Last increment of ctr is above last index
    ctr--;

	// Output in reverse order
    while( ctr >= 0 ){
        rtx_dbug_out_char( output[ctr] );
        ctr--;
    }

    return RTX_SUCCESS;
}


VOID WriteLine(){
    rtx_dbug_outs( (CHAR*)"\r\n" );
}
