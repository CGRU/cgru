#pragma once

#include "cmd.h"

class CmdConfigLoad : public Cmd
{
  public:
	CmdConfigLoad();
	~CmdConfigLoad();
	bool v_processArguments(int argc, char **argv, af::Msg &msg) final;
};
