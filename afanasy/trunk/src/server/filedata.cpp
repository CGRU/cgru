#include "filedata.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

FileData::FileData( char * Data, int Length, const QString & FileName, int Rotate):
   filename( FileName),
   length( Length),
   rotate( Rotate),
   data( NULL)
{
   AFINFA("FileData::FileData: \"%s\" %d bytes R(%d).\n", filename.toUtf8().data(), length, rotate);

   if( Data == NULL)
   {
      AFERROR("FileData::FileData: Data is null.\n");
      return;
   }
   if( length < 0 )
   {
      AFERROR("FileData::FileData: Length is negative.\n");
      return;
   }
   if( length == 0 )
   {
      AFERROR("FileData::FileData: Zero length.\n");
      return;
   }
   if( filename.isEmpty() )
   {
      AFERROR("FileData::FileData: File name is empty.\n");
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
