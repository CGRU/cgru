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
class CmdJobHostsMaximum: public Cmd { public:
   CmdJobHostsMaximum();
   ~CmdJobHostsMaximum();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdJobHostsMask: public Cmd { public:
   CmdJobHostsMask();
   ~CmdJobHostsMask();
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
class CmdJob: public Cmd { public:
   CmdJob();
   ~CmdJob();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
