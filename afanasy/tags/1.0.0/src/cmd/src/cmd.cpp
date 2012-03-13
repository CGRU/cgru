#include "cmd.h"

#include <stdio.h>
#include <stdlib.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

Cmd::Cmd():
   msgtype( 0),
   msgouttype( 0),
   argscount( 0),
   recieving( false)
{
}

Cmd::~Cmd(){}

void Cmd::printInfo() const
{
   printf(" \t%s", cmd.toUtf8().data());
   if( msgtype ) printf(" \t- Msg::%s", af::Msg::TNAMES[msgtype]);
   printf(" \t%s", info.toUtf8().data());
   if( recieving ) printf(" [%s]", af::Msg::TNAMES[msgouttype]);
   printf("\n");
}

void Cmd::printHelp() const
{
   printInfo();
   if( help.isEmpty() == false ) printf( "%s\n", help.toUtf8().data());
}

void Cmd::msgOut( af::Msg& msg) {}
