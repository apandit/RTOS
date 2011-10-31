/*--------------------------------------------------------------------------
 *                      RTX Stub 
 *--------------------------------------------------------------------------
 */
/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */

#include "rtx_inc.h"
#include "rtx.h"
#include "dbug.h"

//kernel codes

// send_message 1
// receive message 2
// request_memory_block 3
// release_memory_block 4
// release_processor 5
// delayed_send 6
// set_process_priority 7
// get_process_priority 8

/* Interprocess Communications*/
int send_message (int process_ID, void * MessageEnvelope)
{

	//return variable
	int returnCode = 0;
	
	//save d0, d1, d2
	asm( "move.l %d0, -(%a7)" );
	asm( "move.l %d1, -(%a7)" );
	asm( "move.l %d2, -(%a7)" );	
	
	//put corect code in the register d0
	asm( "move.l #1, %d0");
	
	//save d1 and place the address of the process_ID to be replaced into d1
	asm( "move.l %0, %%d1" : : "g" (process_ID) );
	//save d1 and place the address of the MessageEnvelope to be replaced into d1
	asm( "move.l %0, %%d2" : : "g" (MessageEnvelope) );
	
	//call trap to go to kernel level primitive
	asm( "trap #0" );
	
	//Check the returnCode stored in D0 to see if the message was sent properly
	asm( "move.l %%d0, %0" : "=g" (returnCode) : );
	
	//restore d2,d1, d0
	asm( "move.l (%a7)+, %d2" );	
	asm( "move.l (%a7)+, %d1" );
	asm( "move.l (%a7)+, %d0" );
	
	return returnCode;
}

void * receive_message(int * sender_ID)
{

	//return variable
	
	//save d0, d1
	asm( "move.l %d0, -(%a7)" );
	asm( "move.l %d1, -(%a7)" );
	
	//variable to update sender_ID
	int returnInt;
	UINT32 returnAddr;
	
	//put corect code in the register d0
	asm( "move.l #2, %d0");
	
	//save d1 and place the address of the MemoryBlock to be replaced into d1
	//asm( "move.l %0, %%d1" : : "g" (address) );
	
	//call trap to go to kernel level primitive
	asm( "trap #0" );
	
	//get return variables
	asm( "move.l %%d1, %0" : "=g" (returnInt) );
	asm( "move.l %%d0, %0" : "=g" (returnAddr) );
	
	//modify the sender_ID
	*sender_ID = returnInt;
	
	//restore d1, d0
	asm( "move.l (%a7)+, %d1" );
	asm( "move.l (%a7)+, %d0" );
	
	return (void *)returnAddr;
}

/*Memory Management*/
void* request_memory_block() 
{

	
	//return variable
	UINT32 blockAddress;
	
	//save d0 and put corect code in the register
	asm( "move.l %d0, -(%a7)" );
	asm( "move.l #3, %d0");
	
	//call trap to go to kernel level primitive
	asm( "trap #0" );
	
	//asm( "trap #5" );
	
	//copy return address to local variable
	asm( "move.l %%d0, %0" : "=g" (blockAddress) : );
	
	//restore d0
	asm( "move.l (%a7)+, %d0" );
	
	//return address of allocated memory block
    return (void *)blockAddress;
}

int release_memory_block(void * MemoryBlock)
{

	//return variable
	int returnVariable = 0;
	//save d0, d1
	asm( "move.l %d0, -(%a7)" );
	asm( "move.l %d1, -(%a7)" );
	
	//put corect code in the register d0
	asm( "move.l #4, %d0");
	
	//save d1 and place the address of the MemoryBlock to be replaced into d1
	asm( "move.l %0, %%d1" : : "g" (MemoryBlock) );
	
	//call trap to go to kernel level primitive
	asm( "trap #0" );
	
	//copy return variable to local variable
	asm( "move.l %%d0, %0" : "=g" (returnVariable) );
	
	//restore d1, d0
	asm( "move.l (%a7)+, %d1" );
	asm( "move.l (%a7)+, %d0" );
	
    return returnVariable;
}

/*Process Management*/
int release_processor()
{

	
	//save do and put correct code in d0
	asm( "move.l %d0, -(%a7)" );
	asm( "move.l #5, %d0" );
	
	//call trap to go to kernel level primitive
	asm( "trap #0" );
	
	//restore d0
	asm( "move.l (%a7)+, %d0" );
    return 0;
}

/*Timing Service*/
int delayed_send(int process_ID, void * MessageEnvelope, int delay)
{

    return 0;
}

/*Process Priority*/
int set_process_priority (int process_ID, int priority)
{

    return 0;
}

int get_process_priority (int process_ID)
{

    return 0;
}
