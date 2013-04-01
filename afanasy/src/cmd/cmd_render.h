#pragma once

#include "cmd.h"

class CmdRenderList : public Cmd { public:
   CmdRenderList();
   ~CmdRenderList();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
};
class CmdRenderResoucesList : public Cmd { public:
   CmdRenderResoucesList();
   ~CmdRenderResoucesList();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
   void v_msgOut( af::Msg& msg);
};
class CmdRenderPriority : public Cmd { public:
   CmdRenderPriority();
   ~CmdRenderPriority();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderNimby : public Cmd { public:
   CmdRenderNimby();
   ~CmdRenderNimby();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderNIMBY: public Cmd { public:
   CmdRenderNIMBY();
   ~CmdRenderNIMBY();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderUser : public Cmd { public:
   CmdRenderUser();
   ~CmdRenderUser();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderFree : public Cmd { public:
   CmdRenderFree();
   ~CmdRenderFree();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderEjectTasks : public Cmd { public:
   CmdRenderEjectTasks();
   ~CmdRenderEjectTasks();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderEjectNotMyTasks : public Cmd { public:
   CmdRenderEjectNotMyTasks();
   ~CmdRenderEjectNotMyTasks();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderExit : public Cmd { public:
   CmdRenderExit();
   ~CmdRenderExit();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderDelete : public Cmd { public:
   CmdRenderDelete();
   ~CmdRenderDelete();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderWOLSleep : public Cmd { public:
   CmdRenderWOLSleep ();
   ~CmdRenderWOLSleep ();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderWOLWake : public Cmd { public:
   CmdRenderWOLWake ();
   ~CmdRenderWOLWake ();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderServiceOn : public Cmd { public:
   CmdRenderServiceOn ();
   ~CmdRenderServiceOn ();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderServiceOff : public Cmd { public:
   CmdRenderServiceOff ();
   ~CmdRenderServiceOff ();
   bool v_processArguments( int argc, char** argv, af::Msg &msg);
};
