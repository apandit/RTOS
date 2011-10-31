/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#ifndef _Queue_
#define _Queue_

#include "pcb.h"
#include "dbug.h"

// Priority Queue definition for processes

typedef struct rtxQueue{
	UINT8 count;
	rtxProcess* head;
	rtxProcess* tail;
} Queue;

// Prototypes for queue
VOID InitQueue( Queue* queue );
VOID Enqueue( Queue* queue, rtxProcess* proc );
rtxProcess* Dequeue( Queue* queue );
rtxProcess* DequeueSpecific( Queue* queue, int pid );

#endif