#pragma once

#include "cmd.h"

class CmdText : public Cmd
{
public:
   CmdText();
   ~CmdText();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdTextGenerate : public Cmd
{
public:
   CmdTextGenerate();
   ~CmdTextGenerate();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
