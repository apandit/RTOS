/**
 * @file: hello.c
 * @brief: hello world sample code. Ouput to janusROM terminal
 * @author: ECE354 Lab Instructors and TAs
 * @author: Irene Huang
 * @date: 2010/05/03
 */
#include "../shared/rtx_inc.h"
#include "dbug.h"

/*
 * gcc expects this function to exist
 */
int __main( void )
{
    return 0;
}

/*
 * Entry point, check with m68k-coff-nm
 */
int main( void )
{

    unsigned int i;
    unsigned long j;

    j = 3;
    j = j * 24567;

    i = 10;
    i = i * 35682824;

    rtx_dbug_outs( (CHAR *) "Hello World!\n\r" );
    return 0;
}
