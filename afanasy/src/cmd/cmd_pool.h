#pragma once

#include "cmd.h"

class CmdPoolList : public Cmd { public:
	CmdPoolList();
	~CmdPoolList();
	bool v_processArguments(int argc, char** argv, af::Msg &msg);
	void v_msgOut(af::Msg& msg);
};
class CmdPoolPriority : public Cmd { public:
	CmdPoolPriority();
	~CmdPoolPriority();
	bool v_processArguments(int argc, char** argv, af::Msg &msg);
};
class CmdPoolPause : public Cmd { public:
	CmdPoolPause();
	~CmdPoolPause();
	bool v_processArguments(int argc, char** argv, af::Msg &msg);
};
class CmdPoolUnpause : public Cmd { public:
	CmdPoolUnpause();
	~CmdPoolUnpause();
	bool v_processArguments(int argc, char** argv, af::Msg &msg);
};
class CmdPoolDelete : public Cmd { public:
	CmdPoolDelete();
	~CmdPoolDelete();
	bool v_processArguments(int argc, char** argv, af::Msg &msg);
};
class CmdPoolServiceAdd : public Cmd { public:
	CmdPoolServiceAdd();
	~CmdPoolServiceAdd();
	bool v_processArguments(int argc, char** argv, af::Msg &msg);
};
class CmdPoolServiceDel : public Cmd { public:
	CmdPoolServiceDel();
	~CmdPoolServiceDel();
	bool v_processArguments(int argc, char** argv, af::Msg &msg);
};
