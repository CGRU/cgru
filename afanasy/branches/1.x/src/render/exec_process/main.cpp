#include <QApplication>

#include "object.h"

#include "../../afjob.h"

int main( int argc, char ** argv)
{
   if( argc < 5)
   {
      printf("\n");
      printf("%s [parser type] [output string] [frames] [command] <arguments...>\n", argv[0]);
      printf("   parser type:\n");
      uint8_t type = 0;
      for( uint8_t t = 0; t < AFJOB::TaskTypesQuantity; t++)
      {
         printf( "      \"%s\"\n", AFJOB::TaskTypeStr[t]);
      }
      printf("   output string: \"printf\" format string.\n");
      printf("\n");
      return 1;
   }
   QApplication app( argc, argv);
   MyObject myObject( argc, argv);
   myObject.show();
   return app.exec();
}
