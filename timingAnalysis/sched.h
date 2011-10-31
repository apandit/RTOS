/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#ifndef _Scheduling_
#define _Scheduling_

#include "rtx_inc.h"
#include "pcb.h"
#include "mem.h"
#include "processmanager.h"
#include "queues.h"
#include "dbug.h"

VOID InitializeScheduler( ProcessManager* procMan );
VOID EnqueueProcess( ProcessManager* procMan, rtxProcess* process, UINT8 queue );
rtxProcess* DequeueProcess( ProcessManager* procMan, UINT8 queue );

VOID ScheduleNextProcess( ProcessManager* procMan, rtxProcess* ptr );


#endif
