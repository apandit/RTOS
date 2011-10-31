/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#ifndef _ProcessControlBlock_
#define _ProcessControlBlock_

#include "rtx_inc.h"
#include "mem.h"

// Define statuses for processes
#define INVALID 0
#define NEW 1
#define RUNNING 2
#define EXIT 3
#define READY 4
#define BLOCKEDMEM 5
#define BLOCKEDMSG 6

#define MAX_NUMPROCS 16
#define PROCSTACKSIZE 256

#define NULLPROCPRIORITY 4

// Context struct
typedef struct rtxProcessContext{
	UINT32 data[8];
	UINT32 address[7];
	UINT32 stackframe[2];
} rtxContext;

// Process Control Block
typedef struct rtxProcess{
    int pid;
    UINT8 status;
    UINT8 priority;

	rtxContext context;
	
	MemoryQueue messageQueue;
	
	void* startAddress;
	void* stackPointer;
	void* next;
} rtxProcess;


rtxProcess* AllocatePCBs( int numOfPCBs );
void* AllocateStack( UINT32 sizeOfStack );
rtxProcess* CreateProcess( rtxProcess* heap, void* startAddress, void* bottomOfStackHeap, int pid, UINT8 priority );
rtxProcess* FindProcess( rtxProcess* heap, int pid );

#endif
