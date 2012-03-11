#pragma once

#include "../libafsql/name_afsql.h"

#include "cmd.h"

class CmdDBCheck : public Cmd { public:
   CmdDBCheck();
   ~CmdDBCheck();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetUsers : public Cmd { public:
   CmdDBResetUsers();
   ~CmdDBResetUsers();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetRenders : public Cmd { public:
   CmdDBResetRenders();
   ~CmdDBResetRenders();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetJobs : public Cmd { public:
   CmdDBResetJobs();
   ~CmdDBResetJobs();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetStat: public Cmd { public:
   CmdDBResetStat();
   ~CmdDBResetStat();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBResetAll : public Cmd { public:
   CmdDBResetAll();
   ~CmdDBResetAll();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBJobsList : public Cmd { public:
   CmdDBJobsList();
   ~CmdDBJobsList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBJobsClean : public Cmd { public:
   CmdDBJobsClean();
   ~CmdDBJobsClean();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBSysJobDel : public Cmd { public:
   CmdDBSysJobDel();
   ~CmdDBSysJobDel();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdDBUpdateTables : public Cmd { public:
   CmdDBUpdateTables();
   ~CmdDBUpdateTables();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
