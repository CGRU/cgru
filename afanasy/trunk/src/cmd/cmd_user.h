#pragma once

#include "cmd.h"

class CmdUserList : public Cmd { public:
   CmdUserList();
   ~CmdUserList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdUserJobsList : public Cmd { public:
   CmdUserJobsList();
   ~CmdUserJobsList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdUserAdd: public Cmd { public:
   CmdUserAdd();
   ~CmdUserAdd();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdUserDelete: public Cmd { public:
   CmdUserDelete();
   ~CmdUserDelete();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdUserPriority : public Cmd { public:
   CmdUserPriority();
   ~CmdUserPriority();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdUserRunningTasksMaximum: public Cmd { public:
   CmdUserRunningTasksMaximum();
   ~CmdUserRunningTasksMaximum();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdUserHostsMask: public Cmd { public:
   CmdUserHostsMask();
   ~CmdUserHostsMask();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
