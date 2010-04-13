#include <Python.h>

#include <stdio.h>

#include <QtCore/QCoreApplication>

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/afattr.h"

#include "../libafnetwork/communications.h"

#include "../libafsql/name_afsql.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#include "afcmd.h"

bool Verbose;
bool Help;
int Protocol;
QString ServerName;
int ServerPort;

int main( int argc, char** argv )
{
   QCoreApplication app( argc, argv);
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
/*
printf("Msg::SizeHeader       = %d\n", Msg::SizeHeader      );
//printf("Msg::SizeDataMin      = %d\n", Msg::SizeDataMin     );
printf("Msg::SizeBuffer       = %d\n", Msg::SizeBuffer      );
printf("Msg::SizeBufferLimit  = %d\n", Msg::SizeBufferLimit );
printf("Msg::SizeDataMax      = %d\n", Msg::SizeDataMax     );
*/
   }
   Verbose = false;
   Help = false;
   Protocol = AF_UNSPEC;
//
// initialize environment variables
   af::Environment ENV( argc == 1 ? af::Environment::Verbose : af::Environment::NoFlags, argc, argv);
   if( ENV.isValid() == false ) return 1;
   ServerName = af::Environment::getServerName().toUtf8().data();
   ServerPort = af::Environment::getServerPort();

   Py_InitializeEx(0);
   if( af::init( af::InitFarm | af::InitServices | (argc == 1 ? af::Verbose : af::NoFlags)) == false) return 1;
   afsql::init();


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
