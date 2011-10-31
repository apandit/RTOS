/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P1-(c) 
 * @date: 2010/05/23
 */
 
#include "pstruct.h"

/*
 * Initialize given process list; stackbase is address at the top of the heap
 */
VOID rtxInitProcessList(rtxProcList* process_list, UINT32 stackbase){
	process_list->count = 0;
	
	// We expect a byte array of size rtxProcHeapSize; thus to find the bottom
	// of the heap, we add the size of byte * the heap size
	process_list->stackbase = stackbase + ( sizeof(BYTE) * rtxProcHeapSize );
	
	// Initialize all processes in the list to pid = 0, and UNINIT status
	int i;
    for(i = 0; i< rtxProcSize; i++){
        process_list->array[i].pid = 0;
		process_list->array[i].status = UNINIT;
    }
}

/*
 * Create process given the PID and address
 */
UINT8 rtxCreateProcess(rtxProcList* process_list, int pid, UINT32 address, UINT8 priority, UINT8 importance ){
    // PID 1 is at array location 0
	UINT8 new_pid = pid-1;
    process_list->array[new_pid].pid = pid;

    // Set relevant process attributes
    process_list->array[new_pid].startAddress = address;
    process_list->array[new_pid].priority = priority;
    process_list->array[new_pid].importance = importance;
    process_list->array[new_pid].status = NEW;
	process_list->array[new_pid].stackPointer = (UINT32)( process_list->stackbase - ( sizeof(BYTE) * 1024 * new_pid));
	
	// Increment the count of processes running
    process_list->count++;

    return pid;
}

/*
 * Not implemented for this milestone.
 */
VOID rtxRemoveProcess(rtxProcList* process_list, UINT8 pid ){

}
