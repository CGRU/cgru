#include "utilities.h"

//Basics:
void inline swapInt( int &a, int &b) {int c = a; a = b; b = c;}

// MIntArray:
bool inMIntArray( int integer, const MIntArray &array, int &index)
{
	int length = array.length();
	for( index = 0; index < length; index++)
		if( integer == array[index])
			return true;
	return false;
}
void sortMIntArray( MIntArray &array)
{
	int length = array.length();
	int i, j;
	for( i = length-1; i > 0; i--)
		for( j = 0; j < i; j++)
			if( array[j] > array[j+1])
				swapInt( array[j], array[j+1]);
}
bool appendMIntArrayOnlyIfNew( MIntArray &array, int integer)
{
	int index;
	if( inMIntArray( integer, array, index)) return false;
	array.append( integer);
	return true;
}
/*
int minMIntArray( MIntArray & intArrayM)
{
	unsigned l = intArrayM.length();
	if( l < 2) return 0;
	int min = intArrayM[0];
	for( unsigned i = 1; i < l; i ++)
		if( intArrayM[i] < min) min = intArrayM[i];
	return min;
}
*/
