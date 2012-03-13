#include "pattern.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Pattern::Pattern( const QString & string):
   ptr_next( NULL),
   name( string),
   description("")
{
}

Pattern::~Pattern()
{
}

bool Pattern::setMask( const QString & string)
{
   mask = string;
   if( mask.isEmpty()) return false;
   regexp.setPattern( mask);
   if( regexp.isValid() == false)
   {
      AFERRAR("Pattern::setMask: \"%s\" set invalid mask \"%s\":\n", name.toUtf8().data(), mask.toUtf8().data());
      printf("%s\n", regexp.errorString().toUtf8().data());
      return false;
   }
   return true;
}

bool Pattern::isValid() const
{
   if( name.isEmpty() )
   {
      AFERROR("Pattern::isValid: name is empty.\n");
      return false;
   }
   if( mask.isEmpty() )
   {
      AFERRAR("Pattern::isValid: \"%s\" mask is empty.\n", name.toUtf8().data());
      return false;
   }
   if( regexp.isValid() == false)
   {
      AFERRAR("Pattern::isValid: \"%s\" mask \"%s\" is invalid:\n", name.toUtf8().data(), mask.toUtf8().data());
//      printf("%s\n", regexp.errorString().toUtf8().data());
      return false;
   }
   return true;
}

void Pattern::stdOut( bool full) const
{
   if( full)
   {
      printf("Pattern: \"%s\" (%s):\n", name.toUtf8().data(), description.toUtf8().data());
      printf("\tMask =\"%s\"\n", mask.toUtf8().data());
   }
   else
   {
      printf("P:\"%s\"-\"%s\": ", mask.toUtf8().data(), name.toUtf8().data());
   }
   host.stdOut( full);
}
