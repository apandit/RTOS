/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#ifndef _UART_H_
#define _UART_H_

#include "rtx_inc.h"

#define UARTPID 14
#define KCDPID 15
#define CRTPID 16
VOID UART_PROCESS();
VOID CRTDisplay();
VOID keyboardCommandDecoder();

#endif
