#include "testprocs.h"
#include "mem.h"
#include "uart.h"
#include "rtx.h"
#include "pcb.h"
#include "dbug.h"

extern MemoryQueue freeMemory;
extern rtxProcess* ProcC;
volatile UINT32 tValue;

VOID ProcessA(){
	rtx_dbug_outs((CHAR*)"ProcA init\r\n");
	int ctr = 0;
	MemoryEnvelope* message;
	//int sender_id;
	int* num;
	rtx_dbug_outs((CHAR*)"ProcA init\r\n");
	while(1){
		rtx_dbug_outs((CHAR*)"Sending Message Number: ");
		WriteNumber(ctr);
		ctr++;
		WriteLine();
		message = (MemoryEnvelope *)request_memory_block();
		message->body[0] = '!';
		message->body[1] = '\0';
		send_message(8,message);
		release_processor();
	}
}

VOID ProcessB(){
	rtx_dbug_outs((CHAR*)"ProcB init\r\n");
	int ctr = 0;
	int sender_id;
	MemoryEnvelope* message = NULL;

	while(1){
		message = receive_message(&sender_id);
		rtx_dbug_outs((CHAR*)"Received Message Number: ");
		WriteNumber(ctr);
		ctr++;
		WriteLine();
		release_processor();
	}
}


VOID ProcessC(){

	rtx_dbug_outs((CHAR*)"ProcC init\r\n");
	int sender_id;
	MemoryEnvelope* message;
	MemoryBlock * block;
	MemoryEnvelope* delayMsg;
	int num = 0;
    int ctr = 0;
    
	while(1){
		release_processor();
	}
	
}
