/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#ifndef _Memory_
#define _Memory_

#include "rtx_inc.h"

#define size_t UINT32
#define MEMHEADER_SIZE 2
#define MEMBLOCK_SIZE 128
#define ENVELOPEBODY_SIZE MEMBLOCK_SIZE - 3*sizeof(int)

typedef struct MemoryEnvelope{
	int senderPid;
	int receiverPid;
	int messageType;
	char body[ENVELOPEBODY_SIZE];
} MemoryEnvelope;

typedef struct MemoryBlock{
	void* next;
	char hack[MEMBLOCK_SIZE];
} MemoryBlock;

typedef struct rtxMemQueue{
	int count;
	MemoryBlock* head;
	MemoryBlock* tail;
} MemoryQueue;

void* malloc( size_t size );
MemoryBlock* AllocateFreeMemory( UINT32 sizeOfBlocks, UINT32 numOfBlocks );
VOID InitializeMemory( MemoryQueue* queue, MemoryBlock* startOfFreeMem, UINT32 numOfBlocks );


// Prototypes for queue
VOID InitMemQueue( MemoryQueue* queue );
VOID EnqueueMem( MemoryQueue* queue, MemoryBlock* block );
MemoryBlock* DequeueMem( MemoryQueue* queue );

#endif

