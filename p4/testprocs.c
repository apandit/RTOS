#include "testprocs.h"
#include "mem.h"
#include "uart.h"
#include "rtx.h"
#include "pcb.h"

extern MemoryQueue freeMemory;
extern rtxProcess* ProcC;

VOID ProcessA(){
	rtx_dbug_outs((CHAR*)"ProcA init\r\n");
	int ctr = 0;
	MemoryEnvelope* message;
	int sender_id;
	int* num;

    // Register %Z command
	message = (MemoryEnvelope*)request_memory_block();
	message->messageType = CMDRegister;
	message->body[0] = 'Z';
	message->body[1] = '\0';
	
	send_message(KCDPID, message);
	message = NULL;


    // Wait for %Z command
	while(1){
		message = (MemoryEnvelope*)receive_message(&sender_id);
		
        //Verify %Z command
		if (message->body[0] == '%' && message->body[1] == 'Z' && message->body[2] == '\0'){
            release_memory_block(message);
	        message = 0;
			break;
		} else {
            release_memory_block( message );
        }
	}
	
	ctr = 0;
	while( 1 ){
		message = (MemoryEnvelope*)request_memory_block();
		message->messageType = count_report;
		
        // Casting magic to write a number to body
        int* num = message->body;
	    *num = ctr;
        
        // Pass message to Process B
		if( send_message( 8, message ) ){
            rtx_dbug_outsl( (CHAR*)"Send message error" );
        }
		ctr++;
		release_processor();
	}

}

VOID ProcessB(){
	rtx_dbug_outs((CHAR*)"ProcB init\r\n");

	int sender_id;
	MemoryEnvelope* message = NULL;

	while(1){
        // Pass message to process C
		message = (MemoryEnvelope*)receive_message(&sender_id);
		send_message(9, message);
        message = NULL;
	}
}


VOID ProcessC(){
	int sender_id;
	MemoryEnvelope* message;
	MemoryBlock * block;
	MemoryEnvelope* delayMsg;
	int num = 0;
    int ctr = 0;
    
	//Local Queue and init
	MemoryQueue localq;
	InitMemQueue(&localq);

	rtx_dbug_outs((CHAR*)"ProcC init\r\n");
	
	while(1){
		// Get messages if local queue is empty (blocking)
		if( localq.count == 0 ){
			//Receive a message from RTX and enqueue onto local queue
			message = (MemoryEnvelope*)receive_message(&sender_id);
		}
		//Dequeue from local message queue
		else{
			block = DequeueMem(&localq);
			message = (MemoryEnvelope *)(block->hack);
			block = NULL;
		}
		
		
		if (message->messageType == count_report){
			//Extract the number from the message
			num = *((int*)message->body);
		    
			if ( num % 20 == 0){
				//Send "Process C" to CRT display, request new message to send
                #ifdef _DEBUG_
				WriteNumber( num );
                WriteLine();
				#endif
				message->body[0] = 'P';
				message->body[1] = 'r';
				message->body[2] = 'o';
				message->body[3] = 'c';
				message->body[4] = 'e';
				message->body[5] = 's';
				message->body[6] = 's';
				message->body[7] = ' ';
				message->body[8] = 'C';
                message->body[9] = '\r';
                message->body[10] = '\n';
				message->body[11] = '\0';
				message->messageType = CRTDisplayRequest;
				send_message(CRTPID,message);
			    

                // Hibernate 10 seconds using delay send
				delayMsg = (MemoryEnvelope*)request_memory_block();
				delayMsg->messageType = wakeup10;
				delayed_send(9,delayMsg,10000);

                
                delayMsg = NULL;
				message = NULL;
				
                ctr = 0;

				while( 1 ){

					delayMsg = (MemoryEnvelope*)receive_message(&sender_id);

					if (delayMsg->messageType == wakeup10){
                        release_memory_block( delayMsg );
                        delayMsg = NULL;
						break;
					}
					//Enqueue Message received into local queue~
					else{
                        block = (MemoryBlock*)((UINT32)delayMsg - sizeof(void *) );
						EnqueueMem(&localq, block);
						delayMsg = NULL;
					}
				}
			}
        } 
       
        if( message != NULL ){
            release_memory_block( message );
            message = NULL;
        }

		release_processor();
	}
}
