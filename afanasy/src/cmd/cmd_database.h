#pragma once

#include "../libafsql/name_afsql.h"

#include "cmd.h"

class CmdDBCheck : public Cmd { public:
   CmdDBCheck();
   ~CmdDBCheck();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetUsers : public Cmd { public:
   CmdDBResetUsers();
   ~CmdDBResetUsers();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetRenders : public Cmd { public:
   CmdDBResetRenders();
   ~CmdDBResetRenders();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetJobs : public Cmd { public:
   CmdDBResetJobs();
   ~CmdDBResetJobs();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetStat: public Cmd { public:
   CmdDBResetStat();
   ~CmdDBResetStat();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetAll : public Cmd { public:
   CmdDBResetAll();
   ~CmdDBResetAll();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBJobsList : public Cmd { public:
   CmdDBJobsList();
   ~CmdDBJobsList();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBJobsClean : public Cmd { public:
   CmdDBJobsClean();
   ~CmdDBJobsClean();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBSysJobDel : public Cmd { public:
   CmdDBSysJobDel();
   ~CmdDBSysJobDel();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBUpdateTables : public Cmd { public:
   CmdDBUpdateTables();
   ~CmdDBUpdateTables();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
