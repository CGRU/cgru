#include "cmd.h"

#include <stdio.h>
#include <stdlib.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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
   std::cout << " \t" << cmd;
   if( msgtype ) std::cout << " \t- Msg::" << af::Msg::TNAMES[msgtype];
   std::cout << " \t" << info;
   if( recieving ) std::cout << " [" << af::Msg::TNAMES[msgouttype] << "]";
   std::cout << std::endl;
}

void Cmd::printHelp() const
{
   printInfo();
   if( false == help.empty() ) std::cout << help << std::endl;
}

void Cmd::msgOut( af::Msg& msg) {}
