	.globl timerAnalysisEntry
	.even		
timerAnalysisEntry:

	move.w #0x2700,%sr
	jsr	TimerIncrement;
    rte
