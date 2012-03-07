#pragma once

#include "cmd.h"

class CmdConfigLoad : public Cmd
{
public:
    CmdConfigLoad();
    ~CmdConfigLoad();
    bool processArguments( int argc, char** argv, af::Msg &msg);
};

class CmdConfigMagic : public Cmd
{
public:
    CmdConfigMagic();
    ~CmdConfigMagic();
    bool processArguments( int argc, char** argv, af::Msg &msg);
};
