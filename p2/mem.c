/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#include "mem.h"

extern void* rtxEnd;

// Allocates space in empty space after end of rtx (__end)
void* malloc( size_t size ){
	void* tmp = rtxEnd;

	// Move the global pointer to the end of the space to allocate
	rtxEnd += size;

	return tmp;
}

// Allocate free memory
MemoryBlock* AllocateFreeMemory( UINT32 sizeOfBlocks, UINT32 numOfBlocks ){
	MemoryBlock* startOfMem = malloc( sizeOfBlocks * numOfBlocks );
	return startOfMem;
}

// Initialize memory by linking together free blocks
VOID InitializeMemory( MemoryQueue* queue, MemoryBlock* startOfMem, UINT32 numOfBlocks ){
	int i;
	for( i = 0; i < numOfBlocks; i++ ){
		EnqueueMem( queue, &startOfMem[i] );
	}
}


//=====================Queue==========================

VOID InitMemQueue( MemoryQueue* queue ){
	queue->count = 0;
	queue->head = NULL;
	queue->tail = NULL;
}

VOID EnqueueMem( MemoryQueue* queue, MemoryBlock* block ){
	// If queue is empty
	if( !queue->count ){
		// Set head and tail to this block
		queue->head = block;
		queue->tail = block;
		queue->count++;
	} else {
		// Put block at the end of the queue
		queue->tail->next = block;
		queue->tail = block;
		queue->count++;
	}
	
	
	// Since we're inserting at end, make sure to clear block->next
	block->next = NULL;
}

MemoryBlock* DequeueMem( MemoryQueue* queue ){
	if( queue->count > 1 ){
		// Take current head, change head to next element
		MemoryBlock* tmp = queue->head;
		queue->head = (MemoryBlock*)queue->head->next;
		queue->count--;
		tmp->next = NULL;
		return tmp;
	} else if( queue->count == 1 ){
		// Take current head, set head & tail to NULL
		MemoryBlock* tmp = queue->head;
		queue->head = queue->tail = NULL;
		queue->count--;
		tmp->next = NULL;
		return tmp;
	} else {
		// Nothing in the queue
		return NULL;
	}
}
