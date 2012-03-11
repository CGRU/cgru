#ifndef utilities_h
#define utilities_h

#include "../definitions.h"

#include <maya/MIntArray.h>

//Basics:
void inline swapInt( int &a, int &b);

//MIntArray:
void sortMIntArray( MIntArray &array);
bool inMIntArray( int integer, const MIntArray &array, int &index);
bool appendMIntArrayOnlyIfNew( MIntArray &array, int integer);

#endif
