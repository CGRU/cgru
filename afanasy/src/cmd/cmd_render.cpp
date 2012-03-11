#include "cmd_render.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/regexp.h"
#include "../libafanasy/render.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern bool Verbose;

CmdRenderList::CmdRenderList()
{
   setCmd("rlist");
   setInfo("List of online Renders.");
   setMsgType( af::Msg::TRendersListRequest);
   setMsgOutType( af::Msg::TRendersList);
   setRecieving();
}
CmdRenderList::~CmdRenderList(){}
bool CmdRenderList::processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( getMsgType());
   return true;
}
void CmdRenderList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.stdOut( Verbose);
}

CmdRenderResoucesList::CmdRenderResoucesList()
{
   setCmd("rrlist");
   setArgsCount(1);
   setInfo("List of resources of Renders specified by mask.");
   setMsgType( af::Msg::TRendersUpdateRequestIds);
   setMsgOutType( af::Msg::TRendersListUpdates);
   setRecieving();
}
CmdRenderResoucesList::~CmdRenderResoucesList(){}
bool CmdRenderResoucesList::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}
void CmdRenderResoucesList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   printf("size=%zu\n", list.getCount());
   for( int i = 0; i < list.getCount(); i++)
   {
      af::Render * render = (af::Render*)(list.getNode(i));
      render->getHostRes().stdOut( false);
   }
}

CmdRenderPriority::CmdRenderPriority()
{
   setCmd("rpri");
   setArgsCount(2);
   setInfo("Change render priority.");
   setHelp("rpri [name] [priority] Set render priority.");
   setMsgType( af::Msg::TRenderSetPriority);
}
CmdRenderPriority::~CmdRenderPriority(){}
bool CmdRenderPriority::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   int priority = atoi(argv[1]);
   af::MCGeneral mcgeneral( name, priority);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderNimby::CmdRenderNimby()
{
   setCmd("rnimby");
   setArgsCount(1);
   setInfo("Set render nimby.");
   setHelp("rnimby [name] Set render to nimby state.");
   setMsgType( af::Msg::TRenderSetNimby);
}
CmdRenderNimby::~CmdRenderNimby(){}
bool CmdRenderNimby::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderNIMBY::CmdRenderNIMBY()
{
   setCmd("rNIMBY");
   setArgsCount(1);
   setInfo("Set render NIMBY.");
   setHelp("rNIMBY [name] Set render to NIMBY state.");
   setMsgType( af::Msg::TRenderSetNIMBY);
}
CmdRenderNIMBY::~CmdRenderNIMBY(){}
bool CmdRenderNIMBY::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderUser::CmdRenderUser()
{
   setCmd("ruser");
   setArgsCount(2);
   setInfo("Set render user.");
   setHelp("ruser [name] [user] Set render for specified user.");
   setMsgType( af::Msg::TRenderSetUser);
}
CmdRenderUser::~CmdRenderUser(){}
bool CmdRenderUser::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   std::string user = argv[1];
   af::MCGeneral mcgeneral( name, user);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderFree::CmdRenderFree()
{
   setCmd("rfree");
   setArgsCount(1);
   setInfo("Set render free.");
   setHelp("rfree [name] Set render free - unset nimby and NIMBY state.");
   setMsgType( af::Msg::TRenderSetFree);
}
CmdRenderFree::~CmdRenderFree(){}
bool CmdRenderFree::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderEjectTasks::CmdRenderEjectTasks()
{
   setCmd("reject");
   setArgsCount(1);
   setInfo("Eject tasks from render.");
   setHelp("reject [name] Eject tasks from specified render.");
   setMsgType( af::Msg::TRenderEjectTasks);
}
CmdRenderEjectTasks::~CmdRenderEjectTasks(){}
bool CmdRenderEjectTasks::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderEjectNotMyTasks::CmdRenderEjectNotMyTasks()
{
   setCmd("rejnotmy");
   setArgsCount(1);
   setInfo("Eject not my tasks from render.");
   setHelp("rejnotmy [name] Eject not my tasks from specified render.");
   setMsgType( af::Msg::TRenderEjectNotMyTasks);
}
CmdRenderEjectNotMyTasks::~CmdRenderEjectNotMyTasks(){}
bool CmdRenderEjectNotMyTasks::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderExit::CmdRenderExit()
{
   setCmd("rexit");
   setInfo("Exit render.");
   setHelp("rexit [name] Exit render with specified name, in no name porivieded, local host name used.");
   setMsgType( af::Msg::TRenderExit);
}
CmdRenderExit::~CmdRenderExit(){}
bool CmdRenderExit::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name( af::Environment::getHostName());
   if( argc > 0) name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderDelete::CmdRenderDelete()
{
   setCmd("rdel");
   setInfo("Delete render.");
   setHelp("rdel [name] Delete render with specified name, in no name porivieded, local host name used.");
   setMsgType( af::Msg::TRenderDelete);
}
CmdRenderDelete::~CmdRenderDelete(){}
bool CmdRenderDelete::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string name( af::Environment::getHostName());
   if( argc > 0) name = argv[0];
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderWOLSleep::CmdRenderWOLSleep()
{
   setCmd("rsleep");
   setInfo("Ask render(s) to sleep.");
   setHelp("rsleep [name] Ask render(s) to sleep.");
   setArgsCount(1);
   setMsgType( af::Msg::TRenderWOLSleep);
}
CmdRenderWOLSleep::~CmdRenderWOLSleep(){}
bool CmdRenderWOLSleep::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string mask = argv[0];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( mask, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderWOLWake::CmdRenderWOLWake()
{
   setCmd("rwake");
   setInfo("Ask sleeping render(s) to wake up.");
   setHelp("rwake [name] Ask render(s) to wake up.");
   setArgsCount(1);
   setMsgType( af::Msg::TRenderWOLWake);
}
CmdRenderWOLWake::~CmdRenderWOLWake(){}
bool CmdRenderWOLWake::processArguments( int argc, char** argv, af::Msg &msg)
{
   std::string mask = argv[0];
   if( af::RegExp::Validate( mask) == false ) return false;
   af::MCGeneral mcgeneral( mask, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}
