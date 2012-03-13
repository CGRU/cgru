#pragma once

#include <QtCore/QString>

#include "../include/afjob.h"
#include "../include/aftypes.h"

#include "../libafanasy/parser.h"

class ParserHost
{

public:

   ParserHost( const QString & task_type, int num_frames);
   ~ParserHost();

   void read( char* output, int size);

   inline bool initialized() const { return init;}

   inline int getPercent()      const { return percent      ;}
   inline int getFrame()        const { return frame        ;}
   inline int getPercentFrame() const { return percentframe ;}

   inline char* getData( int *size ) const { *size = datasize; return data;}

private:
   af::Parser * parser;

   QString type;
   int numframes;

   int  percent;
   int  frame;
   int  percentframe;
   bool error;
   bool warning;

   char*              data;
   int                datasize;
   static const int   DataSizeMax;
   static const int   DataSizeHalf;
   static const int   DataSizeShift;
   bool               overload;
   static const char* overload_string;
   int                overload_string_length;

   bool init;

private:
   void setOverload();
   bool shiftData( int offset);
   void parse( char* output, int size);
};
