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
extern rtxProcess* TIMER;

extern UINT32 timerCounter;

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
			if(receivingProc->priority < rtxProcMan->currentProc->priority ){
                    if( rtxProcMan->interruptProc == NULL ){
    				    EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, READYQUEUE );
                        ScheduleNextProcess( rtxProcMan, receivingProc );
                    } else {
                        EnqueueProcess( rtxProcMan, rtxProcMan->interruptProc, READYQUEUE );
                        rtxProcMan->interruptProc = receivingProc;
                    }
			} else {
				EnqueueProcess( rtxProcMan, receivingProc, READYQUEUE );
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
	MemoryBlock* message = DequeueMem( &(rtxProcMan->currentProc->messageQueue) );

	if(message == NULL){
		//no message available, block
		rtxProcMan->currentProc->status = BLOCKEDMSG;
		ScheduleNextProcess( rtxProcMan, NULL );
	}
	
	//get address of message and sender id
	UINT32 address = (UINT32)(message->hack);
	int sender_ID = ((MemoryEnvelope*)address)->senderPid;

	//put return values in registers
    rtxProcMan->currentProc->context.data[0] = address;
    rtxProcMan->currentProc->context.data[1] = sender_ID;
}

/* Kernel Memory Management*/
void kernel_request_memory_block()
{
	//check if memory is available, block process if no memory available
	if(!(freeMemory.count)){
		EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, MEMORYBLOCKQUEUE );
		ScheduleNextProcess( rtxProcMan, NULL );
	}
	
	//deque memblock
	MemoryBlock * freeMemBlock = DequeueMem(&freeMemory);
	((MemoryEnvelope*)(freeMemBlock->hack))->senderPid = 0;
	((MemoryEnvelope*)(freeMemBlock->hack))->receiverPid = 0;
	((MemoryEnvelope*)(freeMemBlock->hack))->delayCounter = 0;
	((MemoryEnvelope*)(freeMemBlock->hack))->messageType = 0;
	((MemoryEnvelope*)(freeMemBlock->hack))->body[0] = '\0';

	//get address of block
	UINT32 address = (UINT32)((freeMemBlock->hack));	
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
		if(blockedProcess->priority < rtxProcMan->currentProc->priority ){
			if( rtxProcMan->interruptProc == NULL ){
			    EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, READYQUEUE );
                ScheduleNextProcess( rtxProcMan, blockedProcess );
            } else {
                EnqueueProcess( rtxProcMan, rtxProcMan->interruptProc, READYQUEUE );
                rtxProcMan->interruptProc = blockedProcess;
            }
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
	//Dequeue Current process and Enqueue it back onto the ready queue
	EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, READYQUEUE );
	
	//call scheduler
	ScheduleNextProcess( rtxProcMan, NULL );
}


/* Kernel Timing Service*/
void kernel_delayed_send()
{
	int receiverId = 0;
	int delay = 0;
	MemoryEnvelope* messageEnvelope = 0;
	
	//retrieve receiverId and message envelope pointer
	receiverId = (int)rtxProcMan->currentProc->context.data[1];
	messageEnvelope = (MemoryEnvelope*)rtxProcMan->currentProc->context.data[2];
	delay = (int)rtxProcMan->currentProc->context.data[3];

	//fill out header
	messageEnvelope->senderPid = (int)(rtxProcMan->currentProc->pid);
	messageEnvelope->receiverPid = (int)receiverId;
    messageEnvelope->delayCounter = (UINT32)(delay + timerCounter);
	
	//put message in queue of receiving process
	//calculate address of MemoryBlock
	MemoryBlock * block = (MemoryBlock *)((UINT32)messageEnvelope - sizeof(void *));
	
	//find receiving process pcb
	rtxProcess * receivingProc = FindProcess( pcbs, receiverId );
	
	//check if process exists
	if( receivingProc == NULL ){
		//return zero variable
        rtxProcMan->currentProc->context.data[0] = 1;
	} else{
		rtxProcMan->currentProc->context.data[0] = 0;
		SortedEnqueueMem( &(TIMER->messageQueue), block );
	}
}

/* Kernel Process Priority*/
void kernel_set_process_priority (){

	int processId = -1;
	UINT8 priority = 0;
	rtxProcess* proc = NULL;
	rtxProcess* deq = NULL;
	
	//--------------------------------------------------------
	int qCtr;
	rtxProcess* qPtr;
	//------------------------------------------------------
	
	processId = (int)(rtxProcMan->currentProc->context.data[1]);
	priority = (UINT8)( rtxProcMan->currentProc->context.data[2] & 0xFF );
	
	// Error state: cannot change null proc priority or use invalid priority
	if( processId == rtxProcMan->nullProc->pid || priority >= 4 ){

		rtxProcMan->currentProc->context.data[0] = -1;	
	} else {

		proc = FindProcess( pcbs, processId );
		// Couldn't find it; error state
		if( proc == NULL ){

			rtxProcMan->currentProc->context.data[0] = -1;
		} else {
			rtxProcMan->currentProc->context.data[0] = 0;
			
			switch( proc->status ){
				case READY:

					deq = DequeueSpecific( &rtxProcMan->ready[ proc->priority ], processId );

					if( deq != proc ){
						// You did something wrong
						rtx_dbug_outsl( (CHAR*)"Something went horribly wrong..." );
						rtxProcMan->currentProc->context.data[0] = -1;
					} else {
					
						deq->priority = priority;
						
						// Pre-empt?
						if( deq->priority < rtxProcMan->currentProc->priority ){
                            if( rtxProcMan->interruptProc == NULL ){
							    EnqueueProcess( rtxProcMan, rtxProcMan->currentProc, READYQUEUE );
							    ScheduleNextProcess( rtxProcMan, deq );
                            } else {
                                EnqueueProcess( rtxProcMan, rtxProcMan->interruptProc, READYQUEUE );
                                rtxProcMan->interruptProc = deq;
                            }
						} else {
							EnqueueProcess( rtxProcMan, deq, READYQUEUE );
						}
					}
					break;
				case BLOCKEDMEM:
					deq = DequeueSpecific( &rtxProcMan->memBlock[ proc->priority ], processId );
					if( deq != proc ){
						// You did something horribly wrong
						rtx_dbug_outsl( (CHAR*)"Something went horribly wrong v2..." );
						rtxProcMan->currentProc->context.data[0] = -1;
					} else {
						deq->priority = priority;
						EnqueueProcess( rtxProcMan, deq, MEMORYBLOCKQUEUE );
					}
					break;
				case BLOCKEDMSG:
					proc->priority = priority;
					break;
				case RUNNING:
					proc->priority = priority;
					int ctr;
					for( ctr = 0; ctr < (int)priority; ctr++ ){
						if( rtxProcMan->ready[ctr].count ){
                            if( rtxProcMan->interruptProc == NULL ){
							    EnqueueProcess( rtxProcMan, proc, READYQUEUE );
							    ScheduleNextProcess( rtxProcMan, NULL );
                            } else {
                                EnqueueProcess( rtxProcMan, rtxProcMan->interruptProc, READYQUEUE );
                                rtxProcMan->interruptProc = DequeueProcess( rtxProcMan, READYQUEUE );
                                rtxProcMan->interruptProc->status = RUNNING;
                                break;
                            }
						}
					}
					break;
				default:

					// I dun know what to do
					break;
			}
		}
	}
}

void kernel_get_process_priority (){
	int processId = rtxProcMan->currentProc->context.data[1];
	rtxProcess* proc = FindProcess( pcbs, processId );
	if( proc == NULL ){
		rtxProcMan->currentProc->context.data[0] = -1;
	} else {
		rtxProcMan->currentProc->context.data[0] = proc->priority + 0;
	}
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
		
		asm( "move.l %0, %%d0": :"m"(rtxProcMan->currentProc->context.data[0]) );
        asm( "move.l %a6, %a2" );
		asm( "add.l #56, %a2" );
        asm( "move.l %d0, (%a2)" );
    }
    else if( code == 7){
        kernel_set_process_priority();
		
		asm( "move.l %0, %%d0": :"m"(rtxProcMan->currentProc->context.data[0]) );
        asm( "move.l %a6, %a2" );
		asm( "add.l #56, %a2" );
        asm( "move.l %d0, (%a2)" );
    }
    else if( code == 8){
        kernel_get_process_priority();
		
		asm( "move.l %0, %%d0": :"m"(rtxProcMan->currentProc->context.data[0]) );
        asm( "move.l %a6, %a2" );
		asm( "add.l #56, %a2" );
        asm( "move.l %d0, (%a2)" );
    }

    //jump to end
	asm( "unlk %a6" );
	asm( "jmp trap_end" );

}
