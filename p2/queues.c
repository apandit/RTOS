/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#include "queues.h"


// Initialize the queue (set properties to default values)
VOID InitQueue( Queue* queue ){
	queue->count = 0;
	queue->head = NULL;
	queue->tail = NULL;
}

// Insert a value at the end of the queue
VOID Enqueue( Queue* queue, rtxProcess* proc ){
	// If queue is empty
	if( !queue->count ){
		// Set head and tail to this proc
		queue->head = proc;
		queue->tail = proc;
		queue->count++;
	} else {
		// Put proc at the end of the queue
		queue->tail->next = proc;
		queue->tail = proc;
		queue->count++;
	}
	
	
	// Since we're inserting at end, make sure to clear proc->next
	proc->next = NULL;
}

// Take the value at the front of the queue or null if empty
rtxProcess* Dequeue( Queue* queue ){
	
	if( queue->count > 1 ){
		// Take current head, change head to next element
		rtxProcess* tmp = queue->head;
		queue->head = (rtxProcess*)queue->head->next;
		queue->count--;
		tmp->next = NULL;
		return tmp;
	} else if( queue->count == 1 ){
		// Take current head, set head & tail to NULL
		rtxProcess* tmp = queue->head;
		queue->head = queue->tail = NULL;
		queue->count--;
		tmp->next = NULL;
		return tmp;
	} else {
		// Nothing in the queue
		return NULL;
	}
}