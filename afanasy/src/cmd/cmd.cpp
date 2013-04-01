#include "cmd.h"

#include <stdio.h>
#include <stdlib.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Cmd::Cmd():
   m_msgtype( 0),
   m_msgouttype( 0),
   m_argscount( 0),
   m_recieving( false)
{
}

Cmd::~Cmd(){}

void Cmd::v_printInfo() const
{
   std::cout << " \t" << m_cmd;
   if( m_msgtype ) std::cout << " \t- Msg::" << af::Msg::TNAMES[m_msgtype];
   std::cout << " \t" << m_info;
   if( m_recieving ) std::cout << " [" << af::Msg::TNAMES[m_msgouttype] << "]";
   std::cout << std::endl;
}

void Cmd::v_printHelp() const
{
   v_printInfo();
   if( false == m_help.empty() ) std::cout << m_help << std::endl;
}

void Cmd::v_msgOut( af::Msg& msg) {}
