/*----------------------------------------------------------------------------
 *              A Dummy RTX for Testing 
 *----------------------------------------------------------------------------
 */
/**
 * @file: main_rtx.c
 * @author: Thomas Reidemeister 
 * @author: Irene Huang 
 * @author: apandit igrabovi jlfeng ltng
 * @date: 2010.02.18
 * @brief: Example dummy rtx for testing// Initialization
 */

#include "rtx.h"
#include "rtx_test.h"
#include "rtx_inc.h"
#include "dbug.h"
#include "mem.h"
#include "processmanager.h"
#include "pcb.h"
#include "sched.h"
#include "uart.h"
#include "timer.h"
#include "wallclock.h"
#include "priority.h"
#include "testprocs.h"

//#include "io.h"

/* test proc initializaiton info. registration function provided by test suite.
 * test suite needs to register its test proc initilization info with rtx
 * The __REGISTER_TEST_PROCS_ENTRY__ symbol is in linker scripts
 */

extern void __REGISTER_TEST_PROCS_ENTRY__();

volatile MemoryQueue freeMemory;
volatile ProcessManager* rtxProcMan; 
volatile rtxProcess* pcbs;
void* rtxEnd;
extern void* __end;
static volatile int m_nextPid = 0;
extern test_proc_t g_test_proc[NUM_TEST_PROCS];
CHAR testArr[10];

//UART
volatile rtxProcess * CRT;
volatile rtxProcess * KCD;
volatile rtxProcess * UART;


//Timer
volatile rtxProcess * TIMER;
CHAR timerOutput[] = "\02345678901234567890";
extern UINT32 timerCounter;

volatile Command cmd[CMDLIST_SIZE];

//Test procs
rtxProcess* ProcA;
rtxProcess* ProcB;
rtxProcess* ProcC;


/* gcc expects this function to exist */
int __main( void )
{
    return 0;
}

SINT32 coldfire_vbr_init( VOID )
{
	/*
	* Move the VBR into real memory
	*
	* DG: actually, it'll already be here.
	*/
	asm( "move.l %a0, -(%a7)" );
	asm( "move.l #0x10000000, %a0 " );
	asm( "movec.l %a0, %vbr" );
	asm( "move.l (%a7)+, %a0" );
	
	return RTX_SUCCESS;
}


VOID null_process(){
	while( 1 ){
		release_processor();
	}
}


int main() 
{

    rtx_dbug_outs((CHAR *)"rtx: Entering main()\r\n");

    /* get the third party test proc initialization info */
    __REGISTER_TEST_PROCS_ENTRY__();
	
	rtxEnd = &(__end);
	m_nextPid = 1;
	
	// reset counter
	timerCounter = 0;
	
	// Setting up trap # 0
    // Load vector table for trap # 0
    asm( "move.l #asm_trap_entry, %d0" );
    asm( "move.l %d0, 0x10000080" );
	
	#ifdef _DEBUG_
	rtx_dbug_outs( (CHAR*)"Start\r\n" );
	WriteHex((int)malloc(0) );
	rtx_dbug_outs( (CHAR*)" Address before PCBS\r\n" );
	#endif
	// Allocate and initialize pcbs and its stacks
	//rtxProcess* pcbs = AllocatePCBs( MAX_NUMPROCS );
	pcbs = AllocatePCBs( MAX_NUMPROCS );
	#ifdef _DEBUG_
	WriteHex((int)malloc(0) );
	rtx_dbug_outs( (CHAR*)" Address after PCBS\r\n" );
	#endif
	// Initialize the process manager
	rtxProcMan = InitProcessManager();
	#ifdef _DEBUG_
	WriteHex((int)malloc(0) );
	rtx_dbug_outs( (CHAR*)" Address after Procman?\r\n" );
	#endif
	// Create the null process
	rtxProcess* nullProc = CreateProcess( pcbs, null_process, AllocateStack(256), 0, NULLPROCPRIORITY );
	nullProc->status = READY;
	rtxProcMan->nullProc = nullProc;
	#ifdef _DEBUG_
	WriteHex((int)malloc(0) );
	rtx_dbug_outs( (CHAR*)" Address after nullproc?\r\n" );
	#endif
	//Create the Processes
	KCD = CreateProcess( pcbs, &keyboardCommandDecoder, AllocateStack(2048), KCDPID, MEDIUM );
	CRT = CreateProcess( pcbs, &CRTDisplay, AllocateStack(1024), CRTPID, MEDIUM);
	UART = CreateProcess( pcbs, &UART_PROCESS, AllocateStack(1024), UARTPID, HIGH );
	TIMER = CreateProcess( pcbs, &TIMER_PROCESS, AllocateStack(1024), TIMERPID, HIGH );
	rtxProcess* WallP = CreateProcess( pcbs, &WallClock, AllocateStack(1024), WALLPID, HIGH );

    rtxProcess* PriorP = CreateProcess( pcbs, &PriorityProc, AllocateStack(1024), PRIORITYPID, HIGH ); 

	
	// Creating and enqueueing the test processes
	for( m_nextPid = 1 ; m_nextPid < (NUM_TEST_PROCS + 1); m_nextPid++ ){
		EnqueueProcess( 
			rtxProcMan, 
			CreateProcess( 
				pcbs, 
				g_test_proc[m_nextPid-1].entry, 
				AllocateStack( g_test_proc[m_nextPid-1].sz_stack ),
				g_test_proc[m_nextPid-1].pid,
				g_test_proc[m_nextPid-1].priority
			),
			READYQUEUE
		);
	}
	ProcA = CreateProcess( pcbs, &ProcessA, AllocateStack(4096), m_nextPid++, LOW ); 
	ProcB = CreateProcess( pcbs, &ProcessB, AllocateStack(4096), m_nextPid++, LOW ); 
	ProcC = CreateProcess( pcbs, &ProcessC, AllocateStack(4096), m_nextPid++, MEDIUM ); 	

	// Set the next pid to be 1 greater than last test proc pid
	//m_nextPid = g_test_proc[m_nextPid-2].pid + 1;
	
	// Initialize some of our own system processes
	//Testprocs ABC
	EnqueueProcess( rtxProcMan, ProcC, READYQUEUE );
	EnqueueProcess( rtxProcMan, ProcB, READYQUEUE );
	EnqueueProcess( rtxProcMan, ProcA, READYQUEUE );
	
	//System processes
	EnqueueProcess( rtxProcMan, KCD, READYQUEUE );
	EnqueueProcess( rtxProcMan, CRT, READYQUEUE );
	EnqueueProcess( rtxProcMan, WallP, READYQUEUE );
    EnqueueProcess( rtxProcMan, PriorP, READYQUEUE );
	

    if( KCD == NULL || CRT == NULL || WallP == NULL /*|| cp6 == NULL */){
        rtx_dbug_outs( (CHAR*)"Something was null" );
    }
	
	InitCommandArray();
	
	// Initialize the scheduler
	InitializeScheduler( (ProcessManager*)(rtxProcMan) );
	
	// Time to allocate and initialize free memory
	//UINT32 numTotalBlocks = ( (UINT32)0x10200000 - (UINT32)malloc(0)) / sizeof(MemoryBlock);
	UINT32 numTotalBlocks = 30;
	//numTotalBlocks = 1;
	#ifdef _DEBUG_
	WriteHex((int)malloc(0) );
	rtx_dbug_outs( (CHAR*)" Address At start of freemem\r\n" );
	#endif
	// Allocate free memory and create memory table
	MemoryBlock* memstart = AllocateFreeMemory( sizeof(MemoryBlock), numTotalBlocks );
	#ifdef _DEBUG_
	WriteHex((int)malloc(0) );
	rtx_dbug_outs( (CHAR*)" Address after freemem?\r\n" );
	rtx_dbug_outs( (CHAR*)"Allocated Free Memory\r\n" );
	#endif
	InitMemQueue( &freeMemory );
	InitializeMemory( &freeMemory, memstart, numTotalBlocks );
	#ifdef _DEBUG_	
    rtx_dbug_outs( (CHAR*)"Number of free memory blocks: " );
    WriteNumber( freeMemory.count );	
    WriteLine();

	rtx_dbug_outs( (CHAR*)"Initialized Free Memory\r\n" );
	#endif
	
	UINT32 mask;
	
	//disable all interupts 
	asm( "move.w #0x2700,%sr" );
	
	coldfire_vbr_init();
	
	//store the serial ISR at user vector #64
	asm( "move.l #asm_serial_entry,%d0" );
	asm( "move.l %d0,0x10000100" );

    
    //Store the timer ISR at auto-vector #6
    asm( "move.l #asm_timer_entry,%d0" );
    asm( "move.l %d0,0x10000078" );	
	
	
	//reset the entire UART 
	SERIAL1_UCR = 0x10;

	//reset the receiver 
	SERIAL1_UCR = 0x20;
	
	//reset the transmitter 
	SERIAL1_UCR = 0x30;

	//reset the error condition
	SERIAL1_UCR = 0x40;

	//install the interupt
	SERIAL1_ICR = 0x17;
	SERIAL1_IVR = 64;

	//enable interrupts on rx only
	SERIAL1_IMR = 0x02;

	//set the baud rate
	SERIAL1_UBG1 = 0x00;
#ifdef _CFSERVER_           /* add -D_CFSERVER_ for cf-server build */
	SERIAL1_UBG2 = 0x49;    /* cf-server baud rate 19200 */ 
#else
	SERIAL1_UBG2 = 0x92;    /* lab board baud rate 9600 */
#endif 

	//set clock mode
	SERIAL1_UCSR = 0xDD;

	//setup the UART (no parity, 8 bits )
	SERIAL1_UMR = 0x13;
	
	//setup the rest of the UART (noecho, 1 stop bit )
	SERIAL1_UMR = 0x07;

	//setup for transmit and receive
	SERIAL1_UCR = 0x05;
	
	//Setup to use auto-vectored interupt level 6, priority 3
    TIMER0_ICR = 0x9B;

    
    //Set the reference counts, ~1ms
    TIMER0_TRR = 225;
    
    //Setup the timer prescaler and stuff
    TIMER0_TMR = 0xC71B;
	
	//enable interupts
	mask = SIM_IMR;
	mask &= 0x0003dfff;
	SIM_IMR = mask;
	
	rtx_dbug_outs( (CHAR*)"Timer set, UART set\r\n" );
	//enable all interupts
	//asm( "move.w #0x2000,%sr" );
	// end of keyboard interrupts


    rtx_dbug_outs( (CHAR*)"About to start\r\n" );

	// Start it up
	ScheduleNextProcess( rtxProcMan, NULL );
	
    return 0;
}

/* register rtx primitives with test suite */

void  __attribute__ ((section ("__REGISTER_RTX__"))) register_rtx() 
{
    rtx_dbug_outs((CHAR *)"rtx: Entering register_rtx()\r\n");
    g_test_fixture.send_message = send_message;
    g_test_fixture.receive_message = receive_message;
    g_test_fixture.request_memory_block = request_memory_block;
    g_test_fixture.release_memory_block = release_memory_block;
    g_test_fixture.release_processor = release_processor;
    g_test_fixture.delayed_send = delayed_send;
    g_test_fixture.set_process_priority = set_process_priority;
    g_test_fixture.get_process_priority = get_process_priority;
    rtx_dbug_outs((CHAR *)"rtx: leaving register_rtx()\r\n");
}
