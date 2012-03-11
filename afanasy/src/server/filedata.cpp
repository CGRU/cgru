#include "filedata.h"

#include <string.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

FileData::FileData( const char * Data, int Length, const std::string & FileName, int Rotate):
   filename( FileName),
   length( Length),
   rotate( Rotate),
   data( NULL)
{
   AFINFA("FileData::FileData: \"%s\" %d bytes R(%d).", filename.c_str(), length, rotate)

   if( Data == NULL)
   {
      AFERROR("FileData::FileData: Data is null.")
      return;
   }
   if( length < 0 )
   {
      AFERROR("FileData::FileData: Length is negative.")
      return;
   }
   if( length == 0 )
   {
      AFERROR("FileData::FileData: Zero length.")
      return;
   }
   if( filename.size() == 0 )
   {
      AFERROR("FileData::FileData: File name is empty.")
      return;
   }

   data = new char[length];
   if( data == NULL )
   {
      AFERRAR("FileData::FileData: Can't allocate %d bytes for data.", length)
      data = NULL;
      length = 0;
      return;
   }

   memcpy( data, Data, length);
}

FileData::~FileData()
{
   if( data != NULL ) delete [] data;
}
