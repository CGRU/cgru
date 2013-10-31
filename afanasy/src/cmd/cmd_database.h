#pragma once

#include "../libafsql/name_afsql.h"

#include "cmd.h"

class CmdDBCheck : public Cmd { public:
   CmdDBCheck();
   ~CmdDBCheck();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetJobs: public Cmd { public:
   CmdDBResetJobs();
   ~CmdDBResetJobs();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetTasks: public Cmd { public:
   CmdDBResetTasks();
   ~CmdDBResetTasks();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetAll : public Cmd { public:
   CmdDBResetAll();
   ~CmdDBResetAll();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBUpdateTables : public Cmd { public:
   CmdDBUpdateTables();
   ~CmdDBUpdateTables();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
