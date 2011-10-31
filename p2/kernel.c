/*--------------------------------------------------------------------------
 *                      Kernel Stub 
 *--------------------------------------------------------------------------
 */
/**
 * @file:   kernel.c   
 * @author: apandit igrabovi jlfeng ltng 
 * @date:   2010.06.10
 * @brief:  Kernel level primitives
 */
 
#include "rtx_inc.h"
#include "kernel.h"
#include "mem.h"
#include "sched.h"
#include "rtx.h"
#include "processmanager.h"

//external global values
extern MemoryQueue freeMemory;
extern ProcessManager* rtxProcMan; 
extern rtxProcess* pcbs;

/* Kernel Interprocess Communications*/
void kernel_send_message ()
{

	int process_id;
	MemoryEnvelope* messageEnvelope;
	
	//retrieve process_id and message envelope pointer
	process_id = (int)rtxProcMan->currentProc->context.data[1];
	messageEnvelope = (MemoryEnvelope*)rtxProcMan->currentProc->context.data[2];

	
	//fill out header
	messageEnvelope->senderPid = (int)(rtxProcMan->currentProc->pid);
	messageEnvelope->receiverPid = process_id;
	
	//put message in queue of receiving process
	//calculate address of MemoryBlock
	MemoryBlock * block = (MemoryBlock *)((UINT32)messageEnvelope - sizeof(void *));
	
	//find receiving process pcb
	rtxProcess * receivingProc = FindProcess( pcbs, process_id );
	
	//check if process exists
	if( receivingProc == NULL ){
		//return zero variable
        rtxProcMan->currentProc->context.data[0] = 1;
	} else{
        // If the receipient process is blocked waiting for a message
		if( receivingProc->status == BLOCKEDMSG ){

            // Put envelope and sender pid into receipient's context
			receivingProc->context.data[0] = (UINT32)(messageEnvelope);
			receivingProc->context.data[1] = (UINT32)((int)rtxProcMan->currentProc->pid);
			receivingProc->status = READY;

			//put return code in current context
			rtxProcMan->currentProc->context.data[0] = (UINT32)0;
		
            // If the receipient process is higher priority, preempt
			if(receivingProc->priority < rtxProcMan->currentProc->priority 
                    && rtxProcMan->interruptProc == NULL ){
				EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, READYQUEUE );
                ScheduleNextProcess( rtxProcMan, receivingProc );
			} else {
				EnqueueProcess( rtxProcMan, receivingProc, READYQUEUE );
                // If the current process is not an iprocess, schedule appropriately
                // otherwise, return
                if( rtxProcMan->interruptProc == NULL ){
                    ScheduleNextProcess( rtxProcMan, rtxProcMan->currentProc );
                } 
			}
		} else{
			//put message in mailbox queue
			EnqueueMem( &(receivingProc->messageQueue), block );
			
            rtxProcMan->currentProc->context.data[0] = 0;
		}
	}
}

void kernel_receive_message()
{
	//dequeue message
	MemoryBlock * message = DequeueMem( &(rtxProcMan->currentProc->messageQueue) );

	if(message == NULL){
		//no message available, block
		rtxProcMan->currentProc->status = BLOCKEDMSG;
		ScheduleNextProcess( rtxProcMan, NULL );
	}
	
	//get address of message and sender id
	UINT32 address = (UINT32)( &(message->hack[0]) );
	int sender_ID = *( (int *)address );

	//put return values in registers
    rtxProcMan->currentProc->context.data[0] = address;
    rtxProcMan->currentProc->context.data[1] = sender_ID;
}

/* Kernel Memory Management*/
void kernel_request_memory_block()
{
	//check if memory is available, block process if no memory available
	if(!(freeMemory.count)){
        //rtx_dbug_outs((CHAR*)"Out of memory..\r\n" );
		EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, MEMORYBLOCKQUEUE );
		rtxProcMan->currentProc->status = BLOCKEDMEM;
		ScheduleNextProcess( rtxProcMan, NULL );
	}
	
	//deque memblock
	MemoryBlock * freeMemBlock = DequeueMem(&freeMemory);
	
	//get address of block
	UINT32 address = (UINT32)(&(freeMemBlock->hack[0]));
	
    rtxProcMan->currentProc->context.data[0] = address;
}

void kernel_release_memory_block(){
	//retrieve pointer to memory block
	UINT32 address = rtxProcMan->currentProc->context.data[1];
	
	//check if process is waiting for memory
	if( rtxProcMan->memBlockCount ){
		
		//retrieve blocked process and place address of block in d0
		rtxProcess * blockedProcess = DequeueProcess( rtxProcMan, MEMORYBLOCKQUEUE );
		blockedProcess->context.data[0] = address;
		blockedProcess->status = READY;
		
		//put return code in current context
		rtxProcMan->currentProc->context.data[0] = (UINT32)0;
		
		//enque blocked process and current process and call scheduler
		if(blockedProcess->priority < rtxProcMan->currentProc->priority){
			EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, READYQUEUE );
            ScheduleNextProcess( rtxProcMan, blockedProcess );
		}
		else {
			EnqueueProcess( rtxProcMan, blockedProcess, READYQUEUE );
            return; 
		}
	}
	
	//calculate address of MemoryBlock
	MemoryBlock * memory = (MemoryBlock *)(address - sizeof(void *));
	
	//enqueue free memory
	EnqueueMem( &freeMemory, memory);

    rtxProcMan->currentProc->context.data[0] = 0;
}

/* Kernel Process Management*/
void kernel_release_processor()
{
	/*
		Dequeue Current process and Enqueue it back onto the ready queue
	*/
	rtxProcess* ptr = DequeueProcess( rtxProcMan, READYQUEUE );
	EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, READYQUEUE );
	
	//call scheduler
	ScheduleNextProcess( rtxProcMan, ptr );
}


/* Kernel Timing Service*/
void kernel_delayed_send()
{

}

/* Kernel Process Priority*/
void kernel_set_process_priority ()
{

}

void kernel_get_process_priority ()
{

}

void kernel_save_and_trigger ()
{
    //--------------------------------------------------------------------
	//save context in pcb
	asm( "move.l %a7, %a5" );
	asm( "move.l %a6, %a7" );
	
	int i;
	for ( i = 6; i >= 0; i-- ){
		asm ( "move.l (%%a7)+, %0" : "=m" (rtxProcMan->currentProc->context.address[i]) );
	}
	
	for ( i = 7; i >= 0; i--){
		asm( "move.l (%%a7)+, %0" : "=m" (rtxProcMan->currentProc->context.data[i]) );
	}
	
	for ( i = 1; i >= 0; i--){
		asm( "move.l (%%a7)+, %0" : "=m" (rtxProcMan->currentProc->context.stackframe[i]) );
	}
	
	asm( "move.l %%a7, %0" : "=m" (rtxProcMan->currentProc->stackPointer) );
	asm( "move.l %a5, %a7" );
	//--------------------------------------------------------------------

    int code = rtxProcMan->currentProc->context.data[0];
    
    // send_message 1
    // receive message 2
    // request_memory_block 3
    // release_memory_block 4
    // release_processor 5
    // delayed_send 6
    // set_process_priority 7
    // get_process_priority 8

    if( code == 1){
        kernel_send_message();
        
		asm( "move.l %0, %%d0": :"m"(rtxProcMan->currentProc->context.data[0]) );
        asm( "move.l %a6, %a2" );
		asm( "add.l #56, %a2" );
        asm( "move.l %d0, (%a2)" );
    }
    else if( code == 2){
        kernel_receive_message();

	    asm( "move.l %0, %%d1" : : "m" (rtxProcMan->currentProc->context.data[1]) );
	    asm( "move.l %0, %%d0" : : "m" (rtxProcMan->currentProc->context.data[0]) );
        asm( "move.l %a6, %a2" );
	    asm( "add.l #52, %a2" );
        asm( "move.l %d1, (%a2)" );
	    asm( "add.l #4, %a2" );
        asm( "move.l %d0, (%a2)" );
    }
    else if( code == 3 ){
        kernel_request_memory_block();

        //put return address in d0
	    asm( "move.l %0, %%d0" : : "m" (rtxProcMan->currentProc->context.data[0]) );
        asm( "move.l %a6, %a2" );
    	asm( "add.l #56, %a2" );
        asm( "move.l %d0, (%a2)" );
    }
    else if( code == 4 ){
        kernel_release_memory_block();

        //return non-zero variable
	    asm( "move.l %0, %%d0" : : "m" (rtxProcMan->currentProc->context.data[0]) );
	    asm( "move.l %a6, %a2" );
	    asm( "add.l #56, %a2" );
        asm( "move.l %d0, (%a2)" );
    }
    else if( code == 5 ){
        kernel_release_processor();
    }
    else if( code == 6){
        kernel_delayed_send();
    }
    else if( code == 7){
        kernel_set_process_priority();
    }
    else if( code == 8){
        kernel_get_process_priority();
    }

    //jump to end
	asm( "unlk %a6" );
	asm( "jmp trap_end" );

}
