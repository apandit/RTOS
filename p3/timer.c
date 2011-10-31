/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */

#include "rtx.h"
#include "uart.h"
#include "mem.h"
#include "processmanager.h"
#include "rtx_inc.h"
#include "pcb.h"
#include "kernel.h"
#include "sched.h"

extern volatile MemoryQueue freeMemory;
extern volatile ProcessManager* rtxProcMan; 
extern volatile rtxProcess* pcbs;
extern volatile rtxProcess * TIMER;

volatile int receiverId;
volatile rtxProcess* tmpProc;
volatile int timerCounter;

VOID TIMER_PROCESS(){
    
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
	
	/*if (rtxProcMan->currentProc->pid == 7){
		rtx_dbug_outs( (CHAR*)"Testproc1 got interrupted\r\n" );
	}
	else if (rtxProcMan->currentProc->pid == 8){
		rtx_dbug_outs( (CHAR*)"Testproc2 got interrupted\r\n" );
	}
    //*/

	// Acknowledge interrupt
	TIMER0_TER = 0x2;
	
    timerCounter++;

	// Set TIMER as currentProc and save current proc in interruptProc
    rtxProcMan->interruptProc = rtxProcMan->currentProc;
    rtxProcMan->currentProc = TIMER;
    
	MemoryQueue* queue = &(TIMER->messageQueue);
	MemoryBlock* tmp = queue->head;
	
	
	
	// Find all the blocks that have hit their delay and move them
	// to the proper process
	while( queue->count > 0 && ((MemoryEnvelope*)&(queue->head->hack))->delayCounter <= timerCounter ){		
		tmp = DequeueMem( queue );
		receiverId = ((MemoryEnvelope*)&(tmp->hack))->receiverPid;

		tmpProc = FindProcess( pcbs, receiverId );
		if( tmpProc != NULL ){
			// If it is blocked, put it directly into the context so it can return from
			// receive



			if( tmpProc->status == BLOCKEDMSG ){


				tmpProc->context.data[0] = (UINT32)(tmp->hack);
				tmpProc->context.data[1] = (UINT32)((MemoryEnvelope*)&(tmp->hack))->senderPid;
				tmpProc->status = READY;
				EnqueueProcess( rtxProcMan, tmpProc, READYQUEUE );
			} else {



				EnqueueMem( &(tmpProc->messageQueue), tmp );
			}
		} else {


			// release memory block
			rtxProcMan->currentProc->context.data[1] = (UINT32)tmp;
			kernel_release_memory_block();
		}
	}
	
	//Restore currentproc to 
    rtxProcMan->currentProc = rtxProcMan->interruptProc;
    rtxProcMan->interruptProc = NULL;
	
    asm( "unlk %a6" );
    asm( "jmp interrupt_end" );
}
