#include "cmd_config.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdConfigLoad::CmdConfigLoad()
{
	setCmd("cload");
	setInfo("Request server to reload config file.");
	setHelp("cload Request server to reload configuration.");
	setMsgType( af::Msg::TJSON);
}
CmdConfigLoad::~CmdConfigLoad(){}
bool CmdConfigLoad::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	m_str << "{\"reload_config\":{}}";
	return true;
}

