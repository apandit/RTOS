/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#include "pcb.h"

static int m_numProcs = 0;

// Allocates space for all the PCBs at the free space between the end of the kernel and #0x10200000
// Also sets the global allocated # of procs to passed value and initializes each PCB to INVALID
rtxProcess* AllocatePCBs( int numOfPCBs ){
	m_numProcs = numOfPCBs;
	rtxProcess* tmp = (rtxProcess*)( malloc( sizeof(rtxProcess) * numOfPCBs ) );

	int i;
	for( i = 0; i < m_numProcs; i++ ){
        tmp[i].pid = -1;
		tmp[i].status = INVALID;
	}

	return tmp;
}


// sizeof char is 1 byte
void* AllocateStack( UINT32 sizeOfStack ){
	return (void*)( (UINT32)malloc( sizeOfStack ) + sizeOfStack);
}

// Creates a process at the first empty spot in the PCB heap and returns a pointer to it
// If it cannot find an empty spot, it returns NULL
rtxProcess* CreateProcess( rtxProcess* heap, void* startAddress, void* bottomOfStack, int pid, UINT8 priority ){
	int i;
	for( i = 0; i < m_numProcs; i++ ){
		if( heap[i].status == INVALID ){

			heap[i].status = READY;
			heap[i].priority = priority;
			heap[i].pid = pid;
			heap[i].startAddress = startAddress;
			heap[i].next = NULL;
			
			// Initialize message queue
			InitMemQueue( &heap[i].messageQueue );
			
			// Point to the bottom of the stack location we expect
			heap[i].stackPointer = bottomOfStack;
			
			//initialize context
			heap[i].context.stackframe[0] = (UINT32)startAddress;
			heap[i].context.stackframe[1] = (UINT32)(0x40000000);
			
			int j;
			for ( j = 0; j < 8; j++ ){
				if (j == 7){
					heap[j].context.data[j] = 0;
				}
				else{
					heap[j].context.data[j] = 0;
					heap[j].context.address[j] = 0;
				}
			}
			
			return &heap[i];
		}
	}

	return NULL;
}



rtxProcess* FindProcess( rtxProcess* heap, int pid ){
	int i;
	for( i = 0; i < MAX_NUMPROCS; i++){
		if ( heap[i].pid == pid){
			return &heap[i];
		}
	}
	
	return NULL;
}

