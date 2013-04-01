#pragma once

#include "cmd.h"

class CmdNumeric : public Cmd
{
public:
	CmdNumeric();
	~CmdNumeric();
	bool v_processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdNumericGen : public Cmd
{
public:
	CmdNumericGen();
	~CmdNumericGen();
	bool v_processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdNumericCmd : public Cmd
{
public:
	CmdNumericCmd();
	~CmdNumericCmd();
	bool v_processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdNumericCalcTask : public Cmd
{
public:
	CmdNumericCalcTask();
	~CmdNumericCalcTask();
	bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
