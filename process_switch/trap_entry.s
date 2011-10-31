	.globl asm_trap_entry
	.even	

asm_trap_entry:
	move.l %d0, -(%a7)
	move.l %d1, -(%a7)
	move.l %d2, -(%a7)
	move.l %d3, -(%a7)
	move.l %d4, -(%a7)
	move.l %d5, -(%a7)
	move.l %d6, -(%a7)
	move.l %d7, -(%a7)
	move.l %a0, -(%a7)
	move.l %a1, -(%a7)
	move.l %a2, -(%a7)
	move.l %a3, -(%a7)
	move.l %a4, -(%a7)
	move.l %a5, -(%a7)
	move.l %a6, -(%a7)
		
    /* Small hack where we skip link on context switch function */
    /* We also never run rts or unlk since we have an RTE within the function */
	jmp	context_switch+0x4

