#include "dbug.h"
#include "priority.h"
#include "rtx_inc.h"
#include "rtx.h"
#include "mem.h"
#include "uart.h"
#include "strings.h"


typedef enum InputState { SPACE, PID, PID2, SPACET, PRI, INVALID } InputState;

int ParseInput( char* input, int* pid, int* priority ){
    InputState state = SPACE;
	int tmp = 0;

	int retval = -1;
	
	*pid = 0;
	*priority = 0;

	
	if( *input != ' ' ){
		return retval;
	} else {
        state = PID;
    }

    // Iterate to skip space
	input++;
	
	
	while( *input != '\0' ){
	    switch( state ){
            case PID:
                tmp = Ctoi( *input );
                if( tmp >= 0 ){
					state = PID2;
                    *pid = *pid * 10 + tmp;
                } else {
                    *pid = -1;
                    return retval; 
                }
                break;
			case PID2:
				tmp = Ctoi( *input );
                if( *input == ' ' ){
                    state = SPACET;
                } else if( tmp >= 0 ){
                    *pid = *pid * 10 + tmp;
                } else {
                    *pid = -1;
                    return retval; 
                }
                break;
            case SPACET:
                tmp = Ctoi( *input );
                if( tmp >= 0 ){
					*priority = tmp;
                    state = PRI;
                } else {
                    return retval;
                }
                break;
            case PRI:
                tmp = Ctoi( *input );
                if( tmp >= 0 ){
                    *priority = *priority * 10 + tmp;
                } else {
                    *priority = -1;
                    return retval;
                }
                break;
            default:
                return retval;
        }
		input++;
	}
    
    retval = 0;
	return retval;
}


VOID PriorityProc(){
	MemoryEnvelope* message;
	int senderId;
	
	int pid = 0, priority = 0;
	
	message = (MemoryEnvelope*)request_memory_block();
	message->messageType = 3;
	message->body[0] = 'C';
	message->body[1] = '\0';
	
	// Register the command
	send_message( KCDPID, message );
	message = 0;
	
	while(1){
		message = (MemoryEnvelope*)receive_message( &senderId );
		
		if( senderId == KCDPID && GetLength( message->body ) >= 6 ){
			if( ParseInput( &(message->body[2]), &pid, &priority ) ){
				
                if( pid == -1 ){
					CopyString( "\tInvalid pid!\r\n\0", message->body );
					
			    } else if( priority == -1 ){
					CopyString( "\tInvalid priority!\r\n\0", message->body );
					
                } else {
					CopyString( "\tInvalid input format!\r\n\0", message->body );
				}
			} else {
				if( set_process_priority( pid, priority ) ){
					CopyString( "\tSet priority failed!\r\n\0", message->body );
					
				} else {
					CopyString( "\tSuccess!\r\n\0", message->body );
					
				}
			}
			message->messageType = 1;
			send_message( CRTPID, message );
		} else {
			release_memory_block( message );
		}
	}

}
