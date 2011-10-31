/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
 
#include "processmanager.h"

//================================================================================
// Allocate space for everything
#define individualQueueSize sizeof(Queue)
#define processManagerSize ( sizeof(ProcessManager) + PROC_NUMPRIORITIES * individualQueueSize * numProcStateQueues )

// Heap for process manager 
BYTE procHeap[processManagerSize];
void* allocSpace;

void* falloc( int size ){
	void* retval = allocSpace;
	allocSpace += size;
	return retval;
}

//================================================================================
//================================================================================

ProcessManager* InitProcessManager( ){
	allocSpace = &procHeap;
	
	// Set mem location for procman and offset current location for further use
	ProcessManager* retval = falloc( sizeof(ProcessManager) );
	
	// Initialize the process manager with appropriate values;
	// size represents the number of priorities
	retval->readyCount = 0;
	retval->memBlockCount = 0;
	retval->msgBlockCount = 0;
	retval->size = PROC_NUMPRIORITIES;
	retval->nextPid = 1;
	retval->nullProc = NULL;
    retval->interruptProc = NULL;	
	retval->currentProc = NULL;

	
	// Allocate space for queues (4 levels each) and update current location pointer
	
	retval->ready = falloc( PROC_NUMPRIORITIES * sizeof(Queue) );
	retval->memBlock = falloc( PROC_NUMPRIORITIES * sizeof(Queue) );
	
	int i;
	
	// For each queue allocated, initialize queues
	for(i = 0; i < PROC_NUMPRIORITIES; i++ ){
		InitQueue( &(retval->ready[i]) );
		InitQueue( &(retval->memBlock[i]) );		
	}
	
	return retval;
}
