#ifndef _STRING_H_
#define _STRING_H_

#include "rtx_inc.h"

int StartsWith( char* cmpString, char* inputString );
int GetLength( char* inputString );
char* SubString( char* inputString, int startIndex, int length );


#endif