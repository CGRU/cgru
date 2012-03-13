#include "filedata.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

FileData::FileData( char * Data, int Length, const QString & FileName, int Rotate):
   filename( FileName),
   length( Length),
   rotate( Rotate),
   data( NULL)
{
   if( Data == NULL)
   {
      AFERROR("FileData::FileData: Data == NULL\n");
      return;
   }
   if( length < 0 )
   {
      AFERROR("FileData::FileData: length < 0\n");
      return;
   }
   if( length == 0 )
   {
      data = NULL;
      return;
   }

   data = new char[length];
   if( data == NULL )
   {
      AFERRAR("FileData::FileData: Can't allocate %d bytes for data.\n", length);
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
