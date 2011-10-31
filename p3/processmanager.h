/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P1-(c) 
 * @date: 2010/05/23
 */

#ifndef _ProcessManager_
#define _ProcessManager_

#include "rtx_inc.h"
#include "pcb.h"
#include "queues.h"

// Define standard sizes
#define rtxProcSize 16
#define PROC_NUMPRIORITIES 4


// Process Manager

#define numProcStateQueues 2

#define READYQUEUE 1
#define MEMORYBLOCKQUEUE 2

typedef struct rtxProcessManager{
	UINT8 readyCount;
	UINT8 memBlockCount;
	UINT8 msgBlockCount;
	UINT8 size; // # of priority levels
	UINT16 nextPid;
	
	rtxProcess* currentProc;
	rtxProcess* nullProc;

    // Put interrupted process in here
    // If empty, it means process isn't being interrupted by iprocess
    rtxProcess* interruptProc;

	// Arrays of queues (# of queues == PROC_NUMPRIORITIES )
    // 1 queue for each priority level
	Queue* ready;
	Queue* memBlock;
} ProcessManager;

ProcessManager* InitProcessManager();



#endif
