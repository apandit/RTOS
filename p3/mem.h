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
#define ENVELOPEBODY_SIZE (MEMBLOCK_SIZE - 3*sizeof(int))

#define CMDLIST_SIZE 5
#define COMMANDS_STRUCT_SIZE (sizeof(int) + 6*sizeof(char))
#define COMMANDS_SIZE 6

//message types
#define general 0
#define CRTDisplayRequest 1
#define UARTCharacter 2
#define CMDRegister 3
#define CommandCall 6

typedef struct MemoryEnvelope{
	int senderPid;
	int receiverPid;
	int messageType;
	int delayCounter;
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

typedef struct Command{
	int ProcId;
	//size is actually just 5 characters, cause last index must be \0
	char command[COMMANDS_SIZE];
} Command;
/*
typedef struct rtxCommandList{
	int count;
	Command* head;
	Command* tail;
} CommandList;
*/


void* malloc( size_t size );
MemoryBlock* AllocateFreeMemory( UINT32 sizeOfBlocks, UINT32 numOfBlocks );
VOID InitializeMemory( MemoryQueue* queue, MemoryBlock* startOfFreeMem, UINT32 numOfBlocks );


// Prototypes for memory block queue
VOID InitMemQueue( MemoryQueue* queue );
VOID EnqueueMem( MemoryQueue* queue, MemoryBlock* block );
VOID SortedEnqueueMem( MemoryQueue* queue, MemoryBlock* block );
MemoryBlock* DequeueMem( MemoryQueue* queue );

// Prototypes for command list
VOID InitCommandArray();
VOID AddCommand( MemoryBlock* block );
int SearchCommand( char *searchString );


#endif

