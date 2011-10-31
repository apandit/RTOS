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
#include "kernel.h"

extern volatile MemoryQueue freeMemory;
extern volatile ProcessManager* rtxProcMan; 
extern volatile rtxProcess * CRT;
extern volatile rtxProcess * KCD;
extern volatile rtxProcess * UART;
extern volatile rtxProcess * TIMER;
extern volatile UINT32 timerCounter;

CHAR outputArr[32];

volatile UINT8 exitflag = 0;
volatile int uartCounter = 0;
volatile void* Tester = 0;
extern Command cmd[CMDLIST_SIZE];
extern rtxProcess* pcbs;


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

#ifdef _DEBUG_HOTKEYS
	int qCtr;
	int qCtr2;
	rtxProcess* qPtr;
    MemoryBlock* mPtr;
#endif
    if( uartCounter > 0 && uartCounter < ENVELOPEBODY_SIZE  ){

        // Write value to output 
        SERIAL1_WD = ((MemoryEnvelope*)Tester)->body[uartCounter];
        uartCounter++;

        if( ((MemoryEnvelope*)Tester)->body[uartCounter] == '\0' || uartCounter == ENVELOPEBODY_SIZE ){
            uartCounter = -1;
            UART->context.data[1] = Tester;
            kernel_release_memory_block();
            Tester = 0;

            if( UART->messageQueue.count == 0 ){
                SERIAL1_IMR = 2;
            }
        }
    } 
    // Check if there are incoming messages from CRT process
    // and output them if UART is ready to be outputted to
	else if( UART->messageQueue.count != 0 && (temp & 4) ){
        // Reset uart output counter
        uartCounter = 0;

        // Get message from CRT to print
        kernel_receive_message();
        Tester = (void*)UART->context.data[0];
        pid = (int)UART->context.data[1];

		// Write data to output and increment the counter 
		SERIAL1_WD = ((MemoryEnvelope*)Tester)->body[uartCounter++];

        // Check if only a single character (if not keep going)
        if( ((MemoryEnvelope*)Tester)->body[uartCounter] == '\0' ){

            // Reset counter
            uartCounter = -1;

            // Release message memory block
            UART->context.data[1] = Tester;
            kernel_release_memory_block();
            Tester = 0;

            // Check if no other messages waiting; if no messages waiting, disable output
    		if( UART->messageQueue.count == 0 ){
	    		SERIAL1_IMR = 2;
	    	}
        } 
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
				((MemoryEnvelope*)outputMessage)->messageType =	UARTCharacter;
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
				((MemoryEnvelope*)outputMessage)->messageType =	UARTCharacter;
                UART->context.data[1] = KCDPID;
                UART->context.data[2] = outputMessage;
                kernel_send_message();

				outputMessage = NULL;
			}
			
			//Set exitflag so KCD exits, as a full command has been entered
        }
        #ifdef _DEBUG_HOTKEYS
		//! hot key to print ready queue and priorities
		else if ( SERIAL1_RD == 33 ){
            rtx_dbug_outs( (CHAR*)"Processes in Ready Queue: \r\n" );
			
			rtx_dbug_outs( (CHAR*)"Current proc-- PID: " );
			WriteNumber( rtxProcMan->currentProc->pid );
			rtx_dbug_outs( (CHAR*)" --> Priority: " );
			WriteNumber( rtxProcMan->currentProc->priority );
			WriteLine();
			
			rtx_dbug_outs( (CHAR*)"Interrupted proc-- PID: " );
			WriteNumber( rtxProcMan->interruptProc->pid );
			rtx_dbug_outs( (CHAR*)" --> Priority: " );
			WriteNumber( rtxProcMan->interruptProc->priority );
			WriteLine();
			
			for( qCtr = 0; qCtr < rtxProcMan->size; qCtr++){
				rtx_dbug_outs( (CHAR*)"Plvl" );
				WriteNumber( qCtr );
				WriteLine();
				
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
            rtx_dbug_outsl( (CHAR*)"Processes in Memory Blocked Queue: " );
            rtx_dbug_outs( (CHAR*)"Free memory blocks: " );
            WriteNumber( freeMemory.count );
            WriteLine();

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
		
		//$ sign to print registered commands
		else if ( SERIAL1_RD == 36){
            rtx_dbug_outs( (CHAR*)"Registered Commands: \r\n" );
			for( qCtr = 0; qCtr < CMDLIST_SIZE; qCtr++){
				rtx_dbug_outs( (CHAR*)"\tPID: " );
				WriteNumber( cmd[qCtr].ProcId );
				for ( qCtr2 = 0; qCtr2 < 1;qCtr2++)
				{
					rtx_dbug_outs( (CHAR*)" --> Command: " );
					WriteNumber( cmd[qCtr].command[0] );
					WriteLine();
				}
            }
		}
		// ~ sign to print blocked msg processes
		else if( SERIAL1_RD == 126 ){
			rtx_dbug_outsl( (CHAR*)"Blocked on message processes:" );
			for( qCtr = 0; qCtr < MAX_NUMPROCS; qCtr++ ){
				if( pcbs[qCtr].status == BLOCKEDMSG ){
					rtx_dbug_outs( (CHAR*)"\tPID: ");
					WriteNumber( pcbs[qCtr].pid );
					rtx_dbug_outs( (CHAR*)" --> PRIORITY: " );
					WriteNumber( pcbs[qCtr].priority + 0 );
					WriteLine();
				}
			}
		}
        else if( SERIAL1_RD == 95 ){
            rtx_dbug_outs( (CHAR*)"Waiting on timer:" );
            WriteNumber( timerCounter );
            WriteLine();

            mPtr = TIMER->messageQueue.head;
            for( qCtr = 0; qCtr < TIMER->messageQueue.count && mPtr; qCtr++ ){
                rtx_dbug_outs( (CHAR*)"PID: " );
                WriteNumber( ((MemoryEnvelope*)(mPtr->hack))->receiverPid );
                rtx_dbug_outs((CHAR*)" Counter: " );
                WriteNumber( ((MemoryEnvelope*)(mPtr->hack))->delayCounter );
                WriteLine();
                mPtr = mPtr->next;
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
				((MemoryEnvelope*)outputMessage)->messageType =	UARTCharacter;
                
                UART->context.data[1] = KCDPID;
                UART->context.data[2] = outputMessage;
                kernel_send_message();

			}
		}
	}
	//Restore currentproc to 
    rtxProcMan->currentProc = rtxProcMan->interruptProc;
    rtxProcMan->interruptProc = NULL;

    ScheduleNextProcess( rtxProcMan, rtxProcMan->currentProc ); 
}


VOID CRTDisplay (){
	void * msgReceive;
	int pid;

	while (1){
		//Attempt to receive message
		msgReceive = receive_message(&pid);
	
		//Verify the messagetype is the CRT Display request
		if ( ((MemoryEnvelope*)msgReceive)->messageType == 1){
            send_message( UARTPID, msgReceive );
            SERIAL1_IMR = 3;
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

	int k = 0;
	int cmdPid;
	char searchCommand[size];
	
	//Infinitely loop KCD
	while(1){
		//Reset inputCharacters, outChar string
		for (charNum = 0; charNum < size; charNum++){
			inputCharacters[charNum] = '\0';
			if (charNum < 4){
				outChar[charNum] = '\0';
			}
			//searchCommand[charNum] = '\0';
		}
		//Reset counter
		charNum = 0;
		//Set flag used to exit when enter is pressed
		exitflag = 0;

		while( exitflag != 1){
			
			//Receive message
			msgReceive = receive_message(&pid);
            
			if( ((MemoryEnvelope*)msgReceive)->messageType == UARTCharacter){
				switch( (char)(((MemoryEnvelope*)msgReceive)->body[0]) ){
						case '\0':
							break;
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
							break;
						/*
							Special cases for enter key, combining both seperate messages into one for CRT and signalling the end of current input for checking for commands
						*/
						case '\n':
							outChar[0] = '\n';
							outChar[1] = '\0';
							exitflag = 1;
							break;
						case '\r':
							outChar[0] = '\r';
							outChar[1] = '\0';
							break;
						/*
							Update Command string array inputCharacters with character received from UART
							Update outchar with last character typed to send to CRT, and number of characters inputted charNum
						*/
						default:
							inputCharacters[charNum] = (char)(((MemoryEnvelope*)msgReceive)->body[0]);
							outChar[0] = inputCharacters[charNum];
							outChar[1] = '\0';
							//Update the number of characters in Command string array inputCharacters
							charNum++;
							break;
				}
				
				//Replace message received from UART with the message to send to CRTDisplay
				msgSend = msgReceive; 

				//Fill out message to send to CRT process to display
				((MemoryEnvelope*)msgSend)->body[0] = outChar[0];
				((MemoryEnvelope*)msgSend)->body[1] = outChar[1];
				((MemoryEnvelope*)msgSend)->body[2] = outChar[2];
				((MemoryEnvelope*)msgSend)->body[3] = outChar[3];
				//Message Type 1 is CRT display request message type that CRT responds to
				((MemoryEnvelope*)msgSend)->messageType = 1;

				send_message(CRTPID,msgSend);

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
			}
			//Case for command registration, call AddCommand and release memory block once complete
			else if ( ((MemoryEnvelope*)msgReceive)->messageType == CMDRegister ){
				AddCommand(msgReceive);
				release_memory_block(msgReceive);
			}
			//Reset Message variables
			msgReceive = NULL;
			msgSend = NULL;
			pid = 0;
		}
		
		/* 
			Command Call recognized by % sign, search for command
		*/
		if (inputCharacters[0] == '%'){
			//Reset search success variable
			cmdPid = 0;
			/*If command found, create new message and send to destintion with inputcharacters as body
				and messageType == 6
			*/			
			cmdPid = SearchCommand( &inputCharacters );
			if (cmdPid != 0)
			{
				msgSend = request_memory_block();
				((MemoryEnvelope*)msgSend)->messageType = CommandCall;
				//Copy inputCharacters into body of message, to send to the owner of the command
				for (k = 0; k < size; k++){
					((MemoryEnvelope*)msgSend)->body[k] = inputCharacters[k];
				}
				//Send message and reset msgSend variable once complete.
				send_message(cmdPid,msgSend);
				msgSend = NULL;
			}
		}
		release_processor();
	}
	return;
}

