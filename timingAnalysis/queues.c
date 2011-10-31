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
	if( queue->count == 0 ){
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

rtxProcess* DequeueSpecific( Queue* queue, int pid ){	
	rtxProcess* tmp = queue->head;
	if( queue->count > 1 ){
		// Check if the head is what we want
		if( tmp->pid == pid ){
			queue->head = (rtxProcess*)tmp->next;
			queue->count--;
			tmp->next = NULL;
			return tmp;
		}
		
		// Iterate through and get our process
		while( tmp->next ){
			if( ((rtxProcess*)tmp->next)->pid == pid ){
				rtxProcess* ret = (rtxProcess*)tmp->next;
				queue->count--;
				tmp->next = ret->next;
				
				// If we're at the tail, make sure to update it
				if( queue->tail == ret ){
					queue->tail = tmp;
				}
				
				ret->next = NULL;
				return ret;
			}
			
			tmp = tmp->next;
		}
		
		return NULL;
	} else if( queue->count == 1 ){
		if( queue->head->pid == pid ){
			queue->head = queue->tail = NULL;
			queue->count--;
			tmp->next = NULL;
			return tmp;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}
