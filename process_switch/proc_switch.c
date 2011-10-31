/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P1-(c) 
 * @date: 2010/05/23
 */

#include "dbug.h"
#include "pstruct.h"

// Global variables
volatile rtxProcList process_list;
volatile UINT8 swapPid;
volatile UINT8 currentPid;

// Process heap
BYTE proc_stack[rtxProcHeapSize];

/* Prototypes */
VOID process_switch(UINT8 pid); 
VOID proc1(VOID);
VOID proc2(VOID);

/**
 * @param: pid process id of the process that will be switched to
 */

VOID process_switch(UINT8 pid) {
   // Set global nextPid as input
   swapPid = pid;
   
   // Trigger trap
   asm( "TRAP #0" );
}

/* 
 * Handle Trap here
 */
VOID context_switch(){
	
	// Check if a process is currently running
	if(currentPid){
		// Update status and save stack pointer
		process_list.array[currentPid-1].status = READY;
		asm( "move.l %%a7, %0" : "=m" (process_list.array[currentPid-1].stackPointer) );
	}
	
	// New current process
	currentPid = swapPid;
	
    // Couldn't find process; create it
    if( !process_list.array[swapPid-1].status ){
		
		// Initialize process given PID
		if( swapPid == 1 ){
			rtxCreateProcess( &process_list, 1, &proc1, 5, 5 );
		} else {
			rtxCreateProcess( &process_list, 2, &proc2, 5, 6 );
		}
		
		// Update status, swap stack pointers, and insert program counter
		// for exception stack frame
		process_list.array[swapPid-1].status = RUNNING;
		asm("movea.l %0, %%a7": : "m" (process_list.array[swapPid-1].stackPointer) );
		asm("move.l %0, -(%%a7)": : "m" (process_list.array[swapPid-1].startAddress) );
		
		// Create fake SR longword
		asm("move.l #0x40000000, -(%a7)" );
		
		// Initialize saved registers to 0
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		asm("move.l #0x0, -(%a7)");
		
    } else {
		// Update status and swap stack pointers
		process_list.array[swapPid-1].status = RUNNING;
		asm("movea.l %0, %%a7": : "m" (process_list.array[swapPid-1].stackPointer) );
		
    }	

	// Restore saved registers
	asm("move.l (%a7)+, %a6");
	asm("move.l (%a7)+, %a5");
	asm("move.l (%a7)+, %a4");
	asm("move.l (%a7)+, %a3");
	asm("move.l (%a7)+, %a2");
	asm("move.l (%a7)+, %a1");
	asm("move.l (%a7)+, %a0");
	asm("move.l (%a7)+, %d7");
	asm("move.l (%a7)+, %d6");
	asm("move.l (%a7)+, %d5");
	asm("move.l (%a7)+, %d4");
	asm("move.l (%a7)+, %d3");
	asm("move.l (%a7)+, %d2");
	asm("move.l (%a7)+, %d1");
	asm("move.l (%a7)+, %d0");
	
	// Restore exception stack frame (Restores SR and PC; 2 longwords)
	asm("rte");
}



/* 
 * user mode process proc1
 * Do NOT make any change of this function
 */
VOID proc1 () 
{
    int i =0;
    while ( 1) {
      if (i!=0 &&i%5 == 0 ) {
          rtx_dbug_outs((CHAR *) "\n\r");
          process_switch(2); // switch to proc2
      }
      rtx_dbug_out_char('A' + i%26);
      i++;
    } 
}

/* 
 * user process proc2
 * Do NOT make any change of this function
 */
VOID proc2 () 
{
    int i =0;
    while ( 1) {
      if(i!=0 && i%5==0 ) {
          rtx_dbug_outs((CHAR *) "\n\r");
          process_switch(1); // switch to proc1
      }
      rtx_dbug_out_char('a' + i%26);
      i++;
    } 
}


/* gcc expects this function to exist */
int __main( VOID )
{
    return 0;
}

int main( VOID ){
    
	// Set global PID vars
	swapPid = 0;
	currentPid = 0;
	
	// Initialize process list; pass in list and allocated process heap
	rtxInitProcessList( (rtxProcList*)(&process_list), (UINT32)&proc_stack );
	
	// Setting up trap#0
    // Load vector table for trap#0 w/ assembly stuf address
    asm( "move.l #asm_trap_entry, %d0" );
    asm( "move.l %d0, 0x10000080" );

    /* You can add extra code here */
    process_switch(1); // switch to proc1
    return 0;
}
