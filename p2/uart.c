/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */

#include "rtx.h"
#include "uart.h"
#include "mem.h"
#include "processmanager.h"
#include "pcb.h"

extern volatile MemoryQueue freeMemory;
extern volatile ProcessManager* rtxProcMan; 
extern volatile rtxProcess * CRT;
extern volatile rtxProcess * KCD;
extern volatile rtxProcess * UART;

CHAR outputArr[32];

volatile UINT8 exitflag = 0;

VOID UART_PROCESS(){
    //--------------------------------------------------------------------
	//save context in pcb
	asm( "move.l %a7, %a5" );
	asm( "move.l %a6, %a7" );
	
	int i;
	for ( i = 6; i >= 0; i-- ){
		asm ( "move.l (%%a7)+, %0" : "=m" (rtxProcMan->currentProc->context.address[i]) );
	}
	
	for ( i = 7; i >= 0; i--){
		asm( "move.l (%%a7)+, %0" : "=m" (rtxProcMan->currentProc->context.data[i]) );
	}
	
	for ( i = 1; i >= 0; i--){
		asm( "move.l (%%a7)+, %0" : "=m" (rtxProcMan->currentProc->context.stackframe[i]) );
	}
	
	asm( "move.l %%a7, %0" : "=m" (rtxProcMan->currentProc->stackPointer) );
	asm( "move.l %a5, %a7" );
	//--------------------------------------------------------------------

	//Replace currently running process with UART to properly insert Sender ID when sending messages
    rtxProcMan->interruptProc = rtxProcMan->currentProc;
    rtxProcMan->currentProc = UART;
    
	BYTE temp = SERIAL1_UCSR; 
    void* outputMessage;
	int pid;
    void * Tester;

#ifdef _DEBUG_HOTKEYS
	int qCtr;
	rtxProcess* qPtr;
#endif
    // Check if there are incoming messages from CRT process
    // and output them if UART is ready to be outputted to
	if( UART->messageQueue.count != 0 && (temp & 4) ){

        // Get message from CRT to print
        kernel_receive_message();
        Tester = (void*)UART->context.data[0];
        pid = (int)UART->context.data[1];

		// Write data to output  
		SERIAL1_WD = ((MemoryEnvelope*)Tester)->body[0];

		// Disable write
		SERIAL1_IMR = 2;

        // Release message memory block
        UART->context.data[1] = Tester;
        kernel_release_memory_block();

	} 
	//Check if data is ready to be read
	else if (temp & 1) {

		if (SERIAL1_RD == 13) {

			/*	Check for free memory and then send 2 messages to display 
				carriage return and linefeed in 2 messages to CRT
			*/
			if( (freeMemory.count) != 0 ){
                kernel_request_memory_block();
				outputMessage = UART->context.data[0];
				((MemoryEnvelope*)outputMessage)->body[0] = '\r';
				((MemoryEnvelope*)outputMessage)->body[1] = '\0';

                UART->context.data[1] = KCDPID;
                UART->context.data[2] = outputMessage;
                kernel_send_message();

				outputMessage = NULL;
			}

			if( (freeMemory.count) != 0 ){
                kernel_request_memory_block();
				outputMessage = UART->context.data[0];
				((MemoryEnvelope*)outputMessage)->body[0] = '\n';
				((MemoryEnvelope*)outputMessage)->body[1] = '\0';

                UART->context.data[1] = KCDPID;
                UART->context.data[2] = outputMessage;
                kernel_send_message();

				outputMessage = NULL;
			}
			//Set exitflag so KCD exits, as a full command has been entered
			exitflag = 1;
        }
        #ifdef _DEBUG_HOTKEYS
		//! hot key to print ready queue and priorities
		else if ( SERIAL1_RD == 33 ){
            rtx_dbug_outs( (CHAR*)"Processes in Ready Queue: \r\n" );
			for( qCtr = 0; qCtr < rtxProcMan->size; qCtr++){

                qPtr = rtxProcMan->ready[qCtr].head;
                while( qPtr != NULL ){
                    rtx_dbug_outs( (CHAR*)"\tPID: " );
                    WriteNumber( qPtr->pid );
                    rtx_dbug_outs( (CHAR*)" --> Priority: " );
                    WriteNumber( qPtr->priority );
                    WriteLine();

                    qPtr = qPtr->next;
                 }
            }

	    }
	    //@ hot key to print memory blocked queue
		else if ( SERIAL1_RD == 64 ){
            rtx_dbug_outs( (CHAR*)"Processes in Memory Blocked Queue: \r\n" );
			for( qCtr = 0; qCtr < rtxProcMan->size; qCtr++){
                qPtr = rtxProcMan->memBlock[qCtr].head;
                while( qPtr != NULL ){
                    rtx_dbug_outs( (CHAR*)"\tPID: " );
                    WriteNumber( qPtr->pid );
                    rtx_dbug_outs( (CHAR*)" --> Priority: " );
                    WriteNumber( qPtr->priority );
                    WriteLine();

                    qPtr = qPtr->next;
                 }
            }

		}
    	#endif
		
		//Normal case where any non special keys are hit, read the key, request a memory block and send to KCD
		else {
			if( (freeMemory.count) != 0 ){
                
                kernel_request_memory_block();
				outputMessage = UART->context.data[0];
                
				((MemoryEnvelope*)outputMessage)->body[0] = SERIAL1_RD;
				((MemoryEnvelope*)outputMessage)->body[1] = '\0';
				
                
                UART->context.data[1] = KCDPID;
                UART->context.data[2] = outputMessage;
                kernel_send_message();

			}
		}
	}
	//Restore currentproc to 
    rtxProcMan->currentProc = rtxProcMan->interruptProc;
    rtxProcMan->interruptProc = NULL;

    asm( "unlk %a6" );
    asm( "jmp interrupt_end" );
}


VOID CRTDisplay (){
	void * msgReceive;
	void * msgSending;
	int pid;
	int counter;

	while (1){
		//Attempt to receive message
		msgReceive = receive_message(&pid);
        counter = 0;
	
		//Verify the messagetype is the CRT Display request
		if ( ((MemoryEnvelope*)msgReceive)->messageType == 1){

			//Loop through the contents of the message until an end of string character is found
			while( ((MemoryEnvelope*)msgReceive)->body[counter] != '\0' ) {
				//Request memory block to send 1 character to UART to display
				msgSending = request_memory_block();
				((MemoryEnvelope*)msgSending)->body[0] = ((MemoryEnvelope*)msgReceive)->body[counter];
				((MemoryEnvelope*)msgSending)->body[1] = '\0';
				
				//Ensure Message sends, if it fails, release memory block
				if( send_message(UARTPID, msgSending) != 0 ){
					release_memory_block( msgSending );
				}

				SERIAL1_IMR = 3;
				counter++;
			}

			release_memory_block(msgReceive);
		}
		//If Message type is wrong, delete the message, discarding whatever the contents are
		else {
   		        release_memory_block( msgReceive );
        	}

		release_processor();
	}
}

VOID keyboardCommandDecoder(){
	//counter, array, senderID, pointer to message received

	UINT16 charNum = 0;
    int size = 64;
	char inputCharacters[size];
    char outChar[5];

	int pid;

	void * msgReceive;
	void * msgSend;

	//Infinitely loop KCD
	while(1){
		//Reset inputCharacters string
		for (charNum = 0; charNum < size; charNum++){
			inputCharacters[charNum] = '\0';
		}
		//Reset First couple 
		charNum = 0;
		//Set flag used to exit when enter is pressed
		exitflag = 0;

		while( exitflag != 1){
			//Receive message
			msgReceive = receive_message(&pid);
            
			 switch( (char)(((MemoryEnvelope*)msgReceive)->body[0]) ){
					/*
						Detect backspace and remove a character from the inputstring
						Send a message to CRTDisplay and remove the character from the String on the UART1 console
					*/
					case '\b':
					case (char)127:
						if( charNum > 0){
							charNum--;
						}
						inputCharacters[charNum] = '\0';
						outChar[0] = '\b';
						outChar[1] = ' ';
						outChar[2] = '\b';
						outChar[3] = '\0';
						if( charNum > 0){
							charNum--;
						}
						break;
					/*
						Update Command string array inputCharacters with character received from UART
						Update outchar with last character typed to send to CRT
					*/
					default:
						inputCharacters[charNum] = (char)(((MemoryEnvelope*)msgReceive)->body[0]);
						outChar[0] = inputCharacters[charNum];
						outChar[1] = '\0';
						break;
            }
			
			//Replace message received from UART with the message to send to CRTDisplay
            msgSend = msgReceive; 

    		((MemoryEnvelope*)msgSend)->body[0] = outChar[0];
    		((MemoryEnvelope*)msgSend)->body[1] = outChar[1];
    		((MemoryEnvelope*)msgSend)->body[2] = outChar[2];
    		((MemoryEnvelope*)msgSend)->body[3] = outChar[3];
			//Message Type 1 is CRT display request message type that CRT responds to
    		((MemoryEnvelope*)msgSend)->messageType = 1;

			send_message(CRTPID,msgSend);
		
			//Update the number of characters in Command string array inputCharacters
			charNum++;
			/*
				Loop around the inputCharacters if we are outputting too many characters(Capacity 64) before hitting enter
				Send a message to CRT to carriage return
			*/
            if( charNum >= size ){
                charNum = 0;

                msgSend = request_memory_block();

                ((MemoryEnvelope*)msgSend)->body[0] = '\r';
			    ((MemoryEnvelope*)msgSend)->body[1] = '\0';
			    ((MemoryEnvelope*)msgSend)->messageType = 1;

                send_message(CRTPID, msgSend );    
            }
	
			//Reset Message variables
			msgReceive = NULL;
			msgSend = NULL;
			pid = 0;
		}

		//Analyze string here in Part 3
		
		release_processor();
	}
	return;
}

