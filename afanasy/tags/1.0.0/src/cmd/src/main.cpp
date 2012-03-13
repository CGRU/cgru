#include <Python.h>

#include <environment.h>
#include <msg.h>

#include <communications.h>

#include <name_afsql.h>

#include <stdio.h>

#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

#include "afcmd.h"

bool Verbose;
bool Help;

int main( int argc, char** argv )
{
   if ( argc == 1)
   {
      printf("afcmd = '%s'\n", argv[0]);
#ifdef LINUX
      static const int bufsize = 1024;
      char buffer[bufsize];
      char link[] = "/proc/self/exe";
      int linksize = readlink( link, buffer, bufsize);
      QString exe = QString::fromUtf8( buffer, linksize);
      printf("%s='%s'\n", link, exe.toUtf8().data());
#endif
      //GetModuleFileName(NULL, buffer, bufsize);
   }
   Verbose = false;
   Help = false;
//
// initialize environment variables
   af::Environment ENV( argc == 1 ? af::Environment::Verbose : af::Environment::NoFlags, argc, argv);
   if( ENV.isValid() == false ) return 1;

   Py_Initialize();
   if( af::init( af::InitFarm | af::InitServices | (argc == 1 ? af::Verbose : af::NoFlags)) == false) return 1;
   afsql::init();
/*
printf("Msg::SizeHeader       = %d\n", Msg::SizeHeader      );
//printf("Msg::SizeDataMin      = %d\n", Msg::SizeDataMin     );
printf("Msg::SizeBuffer       = %d\n", Msg::SizeBuffer      );
printf("Msg::SizeBufferLimit  = %d\n", Msg::SizeBufferLimit );
printf("Msg::SizeDataMax      = %d\n", Msg::SizeDataMax     );
*/
   int return_value = 0;
//
// initialize command class
   AfCmd afcmd;
//
// generate message from command line arguments
   af::Msg msg;
   if( afcmd.processCommand( argc, argv, msg))
   {
      if( msg.isNull() == false)
      {
         // connect to master
         if( afcmd.connect())
         {
            // send message to master
            if( afcmd.msgSend( msg))
            {
               // recieve message from master if needed
               if( afcmd.isRecieving())
               {
                  af::Msg answer;
                  if( afcmd.msgRecv( answer))
                  {
                     // printf message information
                     afcmd.msgOut( answer);
                  }
                  else  return_value = 1;
               }
            }
            else  return_value = 1;
         }
         else return_value = 1;
      }
   }
   else return_value = 1;

   af::destroy();
   Py_Finalize();

   if( argc == 1) printf("\nType \"afcmd h\" for help.\n");
   return return_value;
}
