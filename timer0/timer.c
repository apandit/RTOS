/**
 * @brief: 5 second timer
 * @author: apandit igrabovi ltng jlfeng
 * @author: ECE354 Staff, TAs, and Irene Huang
 * @date: 2010/05/23
 */

#include "../shared/rtx_inc.h"
#include "dbug.h"

/*
 * Global Variables
 */
SINT32 timer_count = 0;
CHAR timerOutput[] = "\02345678901234567890";

/*
 * gcc expects this function to exist
 */
int __main( void ){
    return 0;
}


/*
 * This function is called by the assembly STUB function
 */
VOID c_timer_handler( VOID ){
    timer_count++;

    // Acknowledge interrupt
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

// Write a single char to SERIAL1
void OutChar( CHAR c ){
	// Check if SERIAL1 is ready to be written to
    while( !(SERIAL1_UCSR & 4) );

	// Write char to SERIAL1
    SERIAL1_WD = c;
}

// Write string to SERIAL1
void OutString( CHAR* s ){
    int ctr = 0;
    while( s[ctr] != '\0' ){
        OutChar( s[ctr] );
        ctr++;
    }
}

// Convert number to string (char array)
// Writes to timerOutput char array
SINT32 BinToDec( int seconds ){
    int md = 0;
    int ctr = 0;

    while( seconds ){
        // Take new least significant digit
        md = seconds % 10;
		
		// Convert to char and place in array
        timerOutput[ctr] = md + '0';
       
        // Remove handled digit
        seconds /= 10;

        ctr++;
    }

	// Last increment of ctr is above last index
    ctr--;

	// Output in reverse order
    while( ctr >= 0 ){
        OutChar( timerOutput[ctr] );
        ctr--;
    }

    OutString( (CHAR*)" seconds passed\n\r" ); 

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
    
    rtx_dbug_outs( (CHAR *) "Timer started\n\r" );
    timer_count = 0;
	
	int seconds_count = 0;

    while ( 1 ){
		// Check that timer is at 5 seconds; reset timer in block
		// or block is evaluated true again (timer doesn't reset fast enough)
		if ( timer_count &&  timer_count % 500 == 0 ) {
            timer_count = 0;
			seconds_count += 5;

            BinToDec( seconds_count );
		}
	};
    return 0;
}
