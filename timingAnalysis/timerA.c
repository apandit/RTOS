#include "timerA.h"
#include "rtx_inc.h"

volatile UINT32 tValue;
volatile UINT32 ctr;

VOID TimerIncrement(){
    // Acknowledge interrupt
    TIMER0_TER = 0x3;

	ctr = 0;
	
	//while (ctr != 50){
	//	ctr++;
	//}
    // Increment value (1us)
    tValue++;
}
