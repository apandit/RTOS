/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P1-(c) 
 * @date: 2010/05/23
 */
 
#include "../shared/rtx_inc.h"

#ifndef _ProcStuff
#define _ProcStuff

// Define standard sizes
#define rtxProcSize 2
#define rtxProcHeapSize rtxProcSize * 1024

// Define statuses for processes
#define UNINIT 0
#define NEW 1
#define RUNNING 2
#define FINISHED 3
#define READY 4
#define BLOCKED 5

#endif

// Process Control Block
typedef struct rtxProcess{
    UINT8 pid;
    UINT8 status;

    UINT8 priority;
    UINT8 importance;

    UINT32 startAddress;
    UINT32 stackPointer;
} rtxProcess;

// List keeps track of processes
typedef struct rtxProcList{
    UINT8 count;
	UINT32 stackbase;
    rtxProcess array[rtxProcSize];
} rtxProcList;

VOID rtxInitProcessList(rtxProcList* process_list, UINT32 stackbase);
UINT8 rtxCreateProcess( rtxProcList* process_list, int pid, UINT32 address, UINT8 priority, UINT8 importance );
VOID rtxRemoveProcess( rtxProcList* process_list, UINT8 pid );