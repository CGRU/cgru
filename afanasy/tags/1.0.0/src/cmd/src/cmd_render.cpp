#include "cmd_render.h"

#include <environment.h>
#include <render.h>
#include <msgclasses/mcgeneral.h>
#include <msgclasses/mcafnodes.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

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
   QString name = QString::fromUtf8(argv[0]);
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}
void CmdRenderResoucesList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   printf("size=%d\n", list.getCount());
   for( int i = 0; i < list.getCount(); i++)
   {
      af::Render * render = (af::Render*)(list.getNode(i));
      render->hres.stdOut( false);
   }
}

CmdRenderPriority::CmdRenderPriority()
{
   setCmd("rpri");
   setArgsCount(2);
   setInfo("Change render priority.");
   setHelp("rpri [name] [priority] Set render priority.");
   setMsgType( af::Msg::TRenderPriority);
}
CmdRenderPriority::~CmdRenderPriority(){}
bool CmdRenderPriority::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   bool ok; int priority = (QString::fromUtf8(argv[1])).toInt( &ok);
   if( ok == false ) return false;
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
   setMsgType( af::Msg::TRenderNimby);
}
CmdRenderNimby::~CmdRenderNimby(){}
bool CmdRenderNimby::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
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
   setMsgType( af::Msg::TRenderNIMBY);
}
CmdRenderNIMBY::~CmdRenderNIMBY(){}
bool CmdRenderNIMBY::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
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
   setMsgType( af::Msg::TRenderUser);
}
CmdRenderUser::~CmdRenderUser(){}
bool CmdRenderUser::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   QString user = QString::fromUtf8(argv[1]);
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
   setMsgType( af::Msg::TRenderFree);
}
CmdRenderFree::~CmdRenderFree(){}
bool CmdRenderFree::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdRenderEject::CmdRenderEject()
{
   setCmd("reject");
   setArgsCount(1);
   setInfo("Eject task from render.");
   setHelp("reject [name] Eject task from specified render.");
   setMsgType( af::Msg::TRenderEject);
}
CmdRenderEject::~CmdRenderEject(){}
bool CmdRenderEject::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
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
   QString name( af::Environment::getHostName());
   if( argc > 0) name = QString::fromUtf8(argv[0]);
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
   QString name( af::Environment::getHostName());
   if( argc > 0) name = QString::fromUtf8(argv[0]);
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}
