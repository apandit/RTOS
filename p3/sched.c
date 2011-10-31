/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#include "sched.h"

extern rtxProcess* CRT;
extern rtxProcess* KCD;

static ProcessManager* m_pman;
extern volatile ProcessManager* rtxProcMan;

VOID InitializeScheduler( ProcessManager* procMan ){
	m_pman = procMan;
}

VOID EnqueueProcess( ProcessManager* procMan, rtxProcess* process, UINT8 queue ){

	// Check which queue we're supposed to enqueue on
	// and enqueue based on the priority of the process
	// then increment the appropriate count
	switch( queue ){
		case READYQUEUE:			
			Enqueue( &(procMan->ready[process->priority]), process );
			procMan->readyCount++;
			break;
		case MEMORYBLOCKQUEUE:
			Enqueue( &(procMan->memBlock[process->priority]), process );
			procMan->memBlockCount++;
			break;
	}
}

rtxProcess* DequeueProcess( ProcessManager* procMan, UINT8 queue ){
	// Choose the correct queue
	Queue* ptr = NULL;
	switch(queue){
		case READYQUEUE:
			if( procMan->readyCount ){
				ptr = procMan->ready;
			}
			break;
		case MEMORYBLOCKQUEUE:
			if( procMan->memBlockCount ){
				ptr = procMan->memBlock;
			}
			break;
	}
	
	if( ptr ){
		
		// Find the highest priority process in the queue
		// and return it. Then decrement the appropriate count.
		int i;
		for( i = 0; i < procMan->size; i++ ){
			if( ptr[i].count ){
				switch(queue){
					case READYQUEUE:
						procMan->readyCount--;
						break;
					case MEMORYBLOCKQUEUE:
						procMan->memBlockCount--;
						break;
				}
				
				return Dequeue( &ptr[i] );
			}
		}
	}
	
	
	return NULL;
}

VOID ScheduleNextProcess( ProcessManager* procMan, rtxProcess* ptr ){

	if( ptr == NULL ){
		ptr = DequeueProcess( procMan, READYQUEUE );
	}
	
	if( ptr ){
		ptr->status = RUNNING;
	} else {
		ptr = procMan->nullProc;
	}
	
	procMan->currentProc = ptr;

	// Change the stack pointer
	asm( "move.l %0, %%a7": : "g"( ptr->stackPointer ) );

	//---------------------------------------
	//restore the context
	int i = 0;
	
	for ( i = 0; i < 2; i++){
		asm( "move.l %0, -(%%a7)" : : "m" (procMan->currentProc->context.stackframe[i]) );
	}
	
	for ( i = 0; i < 8; i++){
		asm( "move.l %0, -(%%a7)" : : "m" (procMan->currentProc->context.data[i]) );
	}
	
	for ( i = 0; i < 7; i++){
		asm( "move.l %0, -(%%a7)" : : "m" (procMan->currentProc->context.address[i]) );
	}
	//----------------------------------------
	
	asm( "move.l (%a7)+, %a6"  );
	asm( "move.l (%a7)+, %a5"  );
	asm( "move.l (%a7)+, %a4"  );
	asm( "move.l (%a7)+, %a3"  );
	asm( "move.l (%a7)+, %a2"  );
	asm( "move.l (%a7)+, %a1"  );
	asm( "move.l (%a7)+, %a0"  );
	asm( "move.l (%a7)+, %d7"  );
	asm( "move.l (%a7)+, %d6"  );
	asm( "move.l (%a7)+, %d5"  );
	asm( "move.l (%a7)+, %d4"  );
	asm( "move.l (%a7)+, %d3"  );
	asm( "move.l (%a7)+, %d2"  );
	asm( "move.l (%a7)+, %d1"  );
	asm( "move.l (%a7)+, %d0"  );

	asm( "rte"  );

}
