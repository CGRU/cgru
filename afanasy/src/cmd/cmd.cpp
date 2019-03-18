#include "cmd.h"

#include <stdio.h>
#include <stdlib.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Cmd::Cmd():
	m_msgtype(0),
	m_msgouttype(0),
	m_argscount(0)
{
}

Cmd::~Cmd(){}

void Cmd::v_printInfo() const
{
	std::cout << "    " << m_cmd;
	int cmd_column_len = 16;
	while(cmd_column_len-- > m_cmd.size())
		std::cout << " ";

	std::cout << " - " << m_info;

	std::cout << std::endl;
}

void Cmd::v_printUsage() const
{
	if (m_usage.empty())
		return;

	std::cout << "Usage: " << m_usage << std::endl;
}

void Cmd::v_printHelp() const
{
	v_printInfo();
	v_printUsage();

	if (m_help.empty())
		return;

	std::cout << m_help << std::endl;
}

void Cmd::v_msgOut( af::Msg& msg) {}
