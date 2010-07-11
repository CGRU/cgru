#include "parserhost.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ParserHost::DataSizeMax   = 1 << 20;
const int ParserHost::DataSizeHalf  = ParserHost::DataSizeMax  >> 1;
const int ParserHost::DataSizeShift = ParserHost::DataSizeHalf >> 1;

const char* ParserHost::overload_string =
"\n\n\n\
   ###########################################################   \n\
   ###   Maximum size reached. Output middle trunctated.   ###   \n\
   ###########################################################   \n\
\n\n\n";

ParserHost::ParserHost( const QString & task_type, int num_frames):
   parser( NULL),
   type( task_type),
   numframes( num_frames),
   percent( 0),
   frame( 0),
   percentframe( 0),
   error( false),
   warning( false),
   badresult( false),
   data( NULL),
   datasize( 0),
   overload( false),

   init( false)
{
   data = new char[DataSizeMax];
   overload_string_length = strlen(overload_string)+1;

   if( data == NULL )
   {
      printf( "ParserHost::ParserHost(): Can`t allocate memory for data.");
      return;
   }

   if( type != "none")
   {
      parser = new af::Parser( type, numframes);
      if( parser->isInitialized() == false)
      {
         delete parser;
         parser = NULL;
      }
   }

   init = true;
}

ParserHost::~ParserHost()
{
   if( parser != NULL) delete parser;
   if( data   != NULL) delete [] data;
}

void ParserHost::read( QByteArray & output)
{
   // return if parser initialization failed
   //
   if( init == false ) return;

   parse( output);
   char* out_data = output.data();
   int   out_size = output.size();

#ifdef AFOUTPUT
printf("\"");for(int c=0;c<out_size;c++)if(out_data[c]>=32)printf("%c", out_data[c]);printf("\":\n");
#endif

   // writing output in buffer
   //
//printf("\nParser::read: size = %d ( datasize = %d )\n", size, datasize);
   char* copy_data = out_data;
   int   copy_size = out_size;
   if( (datasize+output.size()) > DataSizeMax )
   {
//printf("(datasize+size) > DataSizeMax : (%d+%d)>%d\n", datasize, size, DataSizeMax);
      if( datasize < DataSizeHalf )
      {
         memcpy( data+datasize, output, DataSizeHalf-datasize);
         copy_data = out_data + DataSizeHalf - datasize ;
         copy_size = out_size - ( DataSizeHalf - datasize);
         datasize = DataSizeHalf;
      }

      int sizeShift = DataSizeShift;
      if( datasize+copy_size-sizeShift > DataSizeMax ) sizeShift = datasize + copy_size - DataSizeMax;
//printf("sizeShift=%d\n", sizeShift);
      if( sizeShift < datasize-DataSizeHalf ) shiftData( sizeShift);
      else
      {
         copy_data = out_data + copy_size - DataSizeHalf;
         copy_size = DataSizeHalf;
         datasize  = DataSizeHalf;
//printf("sizeShift >= datasize-DataSizeHalf ( %d >= %d-%d )\n", sizeShift, datasize, DataSizeHalf);
      }
      if( overload == false ) setOverload();
   }

//printf("memcpy: datasize=%d, copysize=%d, size=%d\n", datasize, copy_size, size);

   memcpy( data+datasize, copy_data, copy_size);
   datasize += copy_size;

/*#ifdef AFOUTPUT
fflush( stdout);
printf("\n##############################   ParserHost::read: ##############################\n");
fflush( stdout);
::write( 1, data, datasize);
fflush( stdout);
printf("\n#############################################################################\n", datasize);
fflush( stdout);
#endif*/

//printf("end: datasize = %d\n", datasize);
}

bool ParserHost::shiftData( int shift)
{
   if( shift < 0 )
   {
      AFERRAR("ParserHost::shiftData: shift < 0 (%d<0)\n", shift);
      return false;
   }
   if( shift == 0 ) return true;
   memcpy( data+DataSizeHalf, data+DataSizeHalf+shift, datasize-DataSizeHalf-shift);
   datasize -= shift;
   return true;
}

void ParserHost::setOverload()
{
   strcpy( data+DataSizeHalf-overload_string_length, overload_string);
   overload = true;
}

void ParserHost::parse( QByteArray & output)
{
   if( parser )
   {
      bool _warning   = false;
      bool _error     = false;
      bool _badresult = false;
      parser->parse( output, percent, frame, percentframe, _warning, _error, _badresult);
      if ( _error     ) error     = true;
      if ( _warning   ) warning   = true;
      if ( _badresult ) badresult = true;
#ifdef AFOUTPUT
      printf("PERCENT: %d%%", percent);
      printf("; FRAME: %d", frame);
      printf("; PERCENTFRAME: %d%%", percentframe);
      if( _error) printf("; ERROR");
      if( _warning) printf("; WARNING");
      if( _badresult) printf("; BAD RESULT");
      printf("\n");
#endif
   }
}
