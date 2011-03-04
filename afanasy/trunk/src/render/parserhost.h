#pragma once

#include <QtCore/QString>

#include "../include/afjob.h"
#include "../include/aftypes.h"

#include "../libafanasy/parser.h"

class ParserHost
{

public:

   ParserHost( const std::string & task_type, int num_frames);
   ~ParserHost();

   void read( QByteArray & output);

   inline int getPercent()      const { return percent      ;}
   inline int getFrame()        const { return frame        ;}
   inline int getPercentFrame() const { return percentframe ;}
   inline int hasWarning()      const { return warning      ;}
   inline int hasError()        const { return error        ;}
   inline int isBadResult()     const { return badresult    ;}

   inline char* getData( int *size ) const { *size = datasize; return data;}

private:
   af::Parser * parser;

   std::string type;
   int numframes;

   int  percent;
   int  frame;
   int  percentframe;
   bool error;
   bool warning;
   bool badresult;

   char*              data;
   int                datasize;
   static const int   DataSizeMax;
   static const int   DataSizeHalf;
   static const int   DataSizeShift;
   bool               overload;
   static const char* overload_string;
   int                overload_string_length;

private:
   void setOverload();
   bool shiftData( int offset);
   void parse( QByteArray & output);
};
