/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#include "mem.h"
#include "processmanager.h"
#include "dbug.h"

extern void* rtxEnd;
extern Command cmd[CMDLIST_SIZE];
extern ProcessManager* rtxProcMan;

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
    int counter = 0;
    MemoryBlock* tmp;

    block->next = NULL;

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

VOID SortedEnqueueMem( MemoryQueue* queue, MemoryBlock* block ){
	// If queue is empty
	if( !queue->count ){
		queue->head = block;
		queue->tail = block;
		queue->count++;

        block->next = NULL;
	} else {
		MemoryBlock* tmp = queue->head;
		// If head has higher delay than inserted, replace
		if( ((MemoryEnvelope*)(tmp->hack))->delayCounter 
			> ((MemoryEnvelope*)(block->hack))->delayCounter ){
			block->next = tmp;
			queue->head = block;
			queue->count++;
		} else {		
			// Iterate through blocks; find the block w/ higher delayCounter and replace
			while(tmp->next){
				if(((MemoryEnvelope*)(((MemoryBlock*)(tmp->next))->hack))->delayCounter 
					> ((MemoryEnvelope*)(block->hack))->delayCounter ){
					block->next = tmp->next;
					tmp->next = block;
					queue->count++;
					return;
				}
				tmp = tmp->next;
			}
			
			// We only get here if we hit the tail of the queue
			queue->tail->next = block;
			queue->tail = block;
            block->next = NULL;

			queue->count++;
		}
	}
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

//=====================Command List==========================
/*
	Resets and initializes cmd array to all 0s
*/
VOID InitCommandArray()
{
	int i = 0;
	int j;
	for (i = 0; i < CMDLIST_SIZE; i++)
	{
		cmd[i].ProcId = 0;
		j = 0;
		for ( j = 0; j < COMMANDS_SIZE; j++)
		{
			cmd[i].command[j] = '\0';
		}
	}
}

/*
	Used to register command if room available
*/
VOID AddCommand( MemoryBlock* block  )
{	
	int i;
	int m;
	int c;
	for( i = 0; i < COMMANDS_SIZE; i++ )
	{	
		//If command array i is available, add command. 
		if (cmd[i].ProcId == 0)
		{
			m = 0;
			c = 0;
			//Add 1 character to commands
			while ( m < 1 ){
				//Verify first character is a % sign
				if  ( ((MemoryEnvelope*)block)->body[m] == '%'){
					m++;
				}
				//If its a letter, copy over the letter to the command array. Upper case
				else if ( (((MemoryEnvelope*)block)->body[m] > 64) && (((MemoryEnvelope*)block)->body[m] < 91) )
				{
					cmd[i].command[c] = ((MemoryEnvelope*)block)->body[m];	
					m++;
					c++;
				}
				//If its a letter, copy over the letter to the command array. Lowercase
				else if ( (((MemoryEnvelope*)block)->body[m] > 96 ) && (((MemoryEnvelope*)block)->body[m] < 123) )
				{
					cmd[i].command[c] = ((MemoryEnvelope*)block)->body[m];	
					m++;
					c++;
				}		
			}
			//Get Pid form memoryenvelope's sender ID
			cmd[i].ProcId = ((MemoryEnvelope*)block)->senderPid;	
			break;
		}
	}
}

// Returns procID if found, 0 otherwise
int SearchCommand( char *searchString )
{
	int returnPid = 0;
	UINT8 j;
	j = 0;
	//Search through created commands
	while (returnPid == 0 && j < CMDLIST_SIZE)
	{
		//check if the command matches an existing command, return Process ID if matched
		if( cmd[j].command[0] == searchString[1] )
		{
			returnPid = cmd[j].ProcId;
			break;
		}
		j++;
	}
	return returnPid;
}

