#pragma once

#include "cmd.h"

class CmdMonitorList : public Cmd
{
public:
   CmdMonitorList();
   ~CmdMonitorList();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
};

class CmdMonitorLog : public Cmd
{
public:
	CmdMonitorLog();
	~CmdMonitorLog();
	bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
