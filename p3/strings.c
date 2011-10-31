#include "strings.h"

int StartsWith( char* cmpString, char* inputString ){
	int ctr = 0;
	for( ctr = 0; cmpString[ctr] != '\0' && inputString[ctr] != '\0'; ctr++ ){
		if( cmpString[ctr] != inputString[ctr] ){
			return 0;
		}
	}
	
	return 1;
}

int GetLength( char* inputString ){
	int ctr = 0;
	for( ctr = 0; inputString[ctr] != '\0'; ctr++ );
	
	return ctr;
}

char* SubString( char* inputString, int startIndex, int length ){
	int intLength = GetLength( inputString );
	
	if( intLength <= startIndex ){
		return NULL;
	}
	
	if( length <= 0 ){
		return NULL;
	} else if( startIndex + length < intLength ) {
		inputString[startIndex+length] = '\0';
		return &(inputString[startIndex]);
	} else {
		return NULL;
	}
	
}