#pragma once

#include "cmd.h"

class CmdJobsList : public Cmd { public:
   CmdJobsList();
   ~CmdJobsList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdJobsWeight : public Cmd { public:
   CmdJobsWeight();
   ~CmdJobsWeight();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdJobPriority : public Cmd { public:
   CmdJobPriority();
   ~CmdJobPriority();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobRunningTasksMaximum: public Cmd { public:
   CmdJobRunningTasksMaximum();
   ~CmdJobRunningTasksMaximum();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobHostsMask: public Cmd { public:
   CmdJobHostsMask();
   ~CmdJobHostsMask();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobsSetUser: public Cmd { public:
   CmdJobsSetUser();
   ~CmdJobsSetUser();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobId: public Cmd { public:
   CmdJobId();
   ~CmdJobId();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdJobLog: public Cmd { public:
   CmdJobLog();
   ~CmdJobLog();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobProgress: public Cmd { public:
   CmdJobProgress();
   ~CmdJobProgress();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdJobsDelete: public Cmd { public:
   CmdJobsDelete();
   ~CmdJobsDelete();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobsPause: public Cmd { public:
   CmdJobsPause();
   ~CmdJobsPause();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobsStart: public Cmd { public:
   CmdJobsStart();
   ~CmdJobsStart();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobsStop: public Cmd { public:
   CmdJobsStop();
   ~CmdJobsStop();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobsRestart: public Cmd { public:
   CmdJobsRestart();
   ~CmdJobsRestart();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
