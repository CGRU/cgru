#include "afcmd.h"

#include <stdio.h>
#include <stdlib.h>

#include "../libafnetwork/communications.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfCmd::AfCmd():
   socketfd(-1),
   recieving( false),
   command( NULL)
{
   RegisterCommands();
}

AfCmd::~AfCmd()
{
   for( CmdList::iterator it = commands.begin(); it != commands.end(); it++) delete (*it);
   if( socketfd != -1 ) close( socketfd);
}

bool AfCmd::connect()
{
   socketfd = com::connecttomaster( Verbose, Protocol, ServerName.toUtf8().data(), ServerPort);
   if( socketfd == -1)
   {
      printf("AfCmd::connect: can't connect to master.\n");
      return false;
   }
   return true;
}

bool AfCmd::msgSend( af::Msg& msg)
{
   if( Verbose) { printf("AfCmd::msgSend: "); msg.stdOut();}
   if( com::msgsend( socketfd, &msg)) return true;
   printf("AfCmd::msgSend: can't send message to master.\n");
   return false;
}

bool AfCmd::msgRecv( af::Msg& msg)
{
   if(!com::msgread( socketfd, &msg))
   {
      printf("AfCmd::msgSend: reading ansswer failed.\n");
      return false;
   }
   if( Verbose) { printf("AfCmd::msgRecv: "); msg.stdOut();}
   return true;
}

void AfCmd::disconnect()
{
   close(socketfd);
   socketfd = -1;
}

void AfCmd::addCmd( Cmd * cmd)
{
   commands.push_back( cmd);
}

bool AfCmd::processCommand( int argc, char** argv, af::Msg &msg)
{
   QString arg = argv[1];
   if((arg.isNull())||(arg.isEmpty())||(arg=="")) return true;

   bool command_founded = false;
   for( int i = 1; i < argc; i++)
   {
      arg = argv[i];
      for( CmdList::iterator it = commands.begin(); it != commands.end(); it++)
      {
         command = *it;
         if( command->isCmd( argv[i]))
         {
            if( Help)
            {
               command->printHelp();
               return true;
            }
            i++;
            if( command->hasArgsCount( argc - i))
            {
               if( command->processArguments( argc - i, argv + i, msg))
               {
                  command_founded = true;
                  if( msg.isNull())
                  {
                     i += command->getArgsCount();
                     continue;
                  }
                  recieving = command->isRecieving();
                  return true;
               }
            }
            command->printHelp();
            return false;
         }
      }
   }

   if( command_founded && (Help == false)) return true;

   printf("Usage: afcmd type options\n");
   for( CmdList::iterator it = commands.begin(); it != commands.end(); it++)
      (*it)->printInfo();

   printf("Type \"afcmd h [command]\" for command help.\n");

   return false;
}

void AfCmd::msgOut( af::Msg &msg)
{
   if( command->isMsgOutType( msg.type()))
   {
      command->msgOut( msg);
   }
   else
   {
      switch( msg.type())
      {
         case af::Msg::TDATA:
         case af::Msg::TQString:
         case af::Msg::TQStringList:
            msg.stdOutData();
            break;
         case af::Msg::TConfirm:
            printf("Confirm(%d) message recieved.\n", msg.int32());
            break;
         case af::Msg::TNULL:
            printf("None information message.\n");
            break;
         case af::Msg::TInvalid:
            printf("Invalid message.\n");
            break;
         default:
            printf("Unknown (for afcmd) message recieved.\n");
            msg.stdOutData();
            break;
      }
   }
   if( Verbose) { printf("AfCmd::msgOut: "); msg.stdOut();}
}
