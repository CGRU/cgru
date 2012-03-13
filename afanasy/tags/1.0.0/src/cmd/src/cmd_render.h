#pragma once

#include "cmd.h"

class CmdRenderList : public Cmd { public:
   CmdRenderList();
   ~CmdRenderList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdRenderResoucesList : public Cmd { public:
   CmdRenderResoucesList();
   ~CmdRenderResoucesList();
   bool processArguments( int argc, char** argv, af::Msg &msg);
   void msgOut( af::Msg& msg);
};
class CmdRenderPriority : public Cmd { public:
   CmdRenderPriority();
   ~CmdRenderPriority();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderNimby : public Cmd { public:
   CmdRenderNimby();
   ~CmdRenderNimby();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderNIMBY: public Cmd { public:
   CmdRenderNIMBY();
   ~CmdRenderNIMBY();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderUser : public Cmd { public:
   CmdRenderUser();
   ~CmdRenderUser();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderFree : public Cmd { public:
   CmdRenderFree();
   ~CmdRenderFree();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderEject : public Cmd { public:
   CmdRenderEject();
   ~CmdRenderEject();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderExit : public Cmd { public:
   CmdRenderExit();
   ~CmdRenderExit();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
class CmdRenderDelete : public Cmd { public:
   CmdRenderDelete();
   ~CmdRenderDelete();
   bool processArguments( int argc, char** argv, af::Msg &msg);
};
