#include "rtx_inc.h"
#include "rtx.h"
#include "mem.h"
#include "uart.h"
#include "wallclock.h"
#include "strings.h"


//HH:MM:SS
int StringToTime( char* s ){
	int hourFirst = Ctoi(s[0]);
	int hourSecond = Ctoi(s[1]);
	int minFirst = Ctoi(s[3]);
	int minSecond = Ctoi(s[4]);
	int secFirst = Ctoi(s[6]);
	int secSecond = Ctoi(s[7]);

	int correctVal = 	( hourFirst * 10 + hourSecond ) * 3600 +
						( minFirst * 10 + minSecond ) * 60 +
						( secFirst * 10 + secSecond );
	if( s[2] != ':' || s[5] != ':' || s[8] != '\0' || correctVal >= 86400 
		|| hourFirst == -1 || hourSecond == -1 || minFirst == -1 || minSecond == -1 || secFirst == -1 || secSecond == -1 
		|| hourFirst >= 3 || (hourFirst == 2 && hourSecond >= 5) || minFirst >= 6 || secFirst >= 6){
		return -1;
	} else {
		return correctVal;
	}
}

VOID SpitOutTime( int time ){

	int hrs = time / 3600;
	int mins = (time % 3600 ) / 60;
	int seconds = time % 60;
	
	MemoryEnvelope* msg = (MemoryEnvelope*)request_memory_block();
	msg->messageType = 1;
	
	int ctr = 0;
	
	// ESC[s - save cursor position
	msg->body[ctr++] = 27; // ESC
	msg->body[ctr++] = '[';
	msg->body[ctr++] = 's';
	// ESC[0;50 - move to position: line 0, column 50
	msg->body[ctr++] = 27; // ESC
	msg->body[ctr++] = '[';
	msg->body[ctr++] = '0';
	msg->body[ctr++] = ';';
	msg->body[ctr++] = '6';
	msg->body[ctr++] = '6';
	msg->body[ctr++] = 'H';
	// "Time: "
	msg->body[ctr++] = 'T';
	msg->body[ctr++] = 'i';
	msg->body[ctr++] = 'm';
	msg->body[ctr++] = 'e';
	msg->body[ctr++] = ':';
	msg->body[ctr++] = ' ';
	
	/* 
		format and print out the time
	*/
	if( hrs >= 10 ){
		msg->body[ctr++] = (CHAR)(hrs / 10 + '0');
		msg->body[ctr++] = (CHAR)(hrs % 10 + '0');
	} else {
		msg->body[ctr++] = '0';
		msg->body[ctr++] = (CHAR)(hrs + '0');
	}
	
	msg->body[ctr++] = ':';
	
	if( mins >= 10 ){
		msg->body[ctr++] = (CHAR)(mins / 10 + '0');
		msg->body[ctr++] = (CHAR)(mins % 10 + '0');
	} else {
		msg->body[ctr++] = '0';
		msg->body[ctr++] = (CHAR)(mins + '0');	
	}
	
	msg->body[ctr++] = ':';
	
	if( seconds >= 10 ){
		msg->body[ctr++] = (CHAR)(seconds / 10 + '0');
		msg->body[ctr++] = (CHAR)(seconds % 10 + '0');
	} else {
		msg->body[ctr++] = '0';
		msg->body[ctr++] = (CHAR)(seconds + '0');
	}
	
	// ESC[u - restore cursor position
	msg->body[ctr++] = 27; // ESC
	msg->body[ctr++] = '[';
	msg->body[ctr++] = 'u';
	
	msg->body[ctr++] = '\0';
	
	//Send message to CRT to display
	send_message( CRTPID, msg );
}

VOID ClearTime(){
	
	MemoryEnvelope* msg = (MemoryEnvelope*)request_memory_block();
	msg->messageType = 1;
	
	int ctr = 0;
	
	// ESC[s - save cursor position
	msg->body[ctr++] = 27; // ESC
	msg->body[ctr++] = '[';
	msg->body[ctr++] = 's';
	// ESC[0;50 - move to position: line 0, column 50
	msg->body[ctr++] = 27; // ESC
	msg->body[ctr++] = '[';
	msg->body[ctr++] = '0';
	msg->body[ctr++] = ';';
	msg->body[ctr++] = '6';
	msg->body[ctr++] = '6';
	msg->body[ctr++] = 'H';
	// "Time: "
	int ct2 = 0;
	for( ct2 = 0; ct2 < 14; ct2++ ){
		msg->body[ctr++] = ' ';
	}	
	
	// ESC[u - restore cursor position
	msg->body[ctr++] = 27; // ESC
	msg->body[ctr++] = '[';
	msg->body[ctr++] = 'u';
	
	msg->body[ctr++] = '\0';
	
	//Send message to CRT to display and clear the time display
	send_message( CRTPID, msg );
}

VOID WallClock(){
	MemoryEnvelope* message;
	int senderId;
	int wTime = -1;

	//Register command by sending message with type CMDRegister to KCD
	message = (MemoryEnvelope*)request_memory_block();
	message->messageType = CMDRegister;
	message->body[0] = 'W';
	message->body[1] = '\0';
	
	send_message( KCDPID, message );
	message = 0;
	
	// %W is registered
	// %WS HH:MM:SS is set
	// %WT is terminate
	while(1){
		message = (MemoryEnvelope*)receive_message( &senderId );
		
		//Verify the sender is from KCD
		if( senderId == KCDPID /*message->messageType != 42*/ && GetLength( message->body ) > 2 ){
			//Check if the command is to start the clock with a time
			if( StartsWith( (CHAR*)"%WS ", message->body ) ){
				
				//Clock is currently running
				if( wTime != -1 ){
					
					wTime = StringToTime( &(message->body[4]) );
					//New inputted time is invalid, so wipe the current time away
					if( wTime == -1 ){
						ClearTime();
					} 
					//New time is valid, replace current time
					else {
						SpitOutTime( wTime );
					}
					release_memory_block( message );
					continue;
				}
				
				//wTime = StringToTime( SubString( message->body, 4, 9) );
				wTime = StringToTime( &(message->body[4]) );

				if( wTime != -1 ){
				
					// Send 1s later, to update time
					message->messageType = 42;
					delayed_send( WALLPID, message, 1000 );
					
					// Spit out time
					SpitOutTime( wTime );
				}
				
			} 
			//Terminate clock when exactly '%WT' detected
			else if( StartsWith( (CHAR*)"%WT", message->body ) && GetLength( message->body ) == 3 ){
				wTime = -1;
				ClearTime();
				release_memory_block( message );
			}
			//Unknown command, release message
			else {
				release_memory_block( message );
			}
		} 
		//Clock update by 1 second, delay send a new message to itself(Wallclock)
		else if( message->messageType == 42 ){
			//Current time is running, resend msg and update clock
			if( wTime != -1 ){
				wTime = wTime + 1 == 86400 ? 0 : wTime + 1;
				delayed_send( WALLPID, message, 1000 );
				SpitOutTime( wTime );
			} 
			//Clock isn't running, nothing to update just release message
			else {
				release_memory_block( message );
			}
		} else {
			// Just release memory since this is error state
			release_memory_block( message );
		}
	}
}

