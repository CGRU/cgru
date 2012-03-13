#include "cmd_user.h"

#include <msgclasses/mcgeneral.h>
#include <msgclasses/mcafnodes.h>

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

extern bool Verbose;

CmdUserList::CmdUserList()
{
   setCmd("ulist");
   setInfo("List of permanent users.");
   setMsgType( af::Msg::TUsersListRequest);
   setMsgOutType( af::Msg::TUsersList);
   setRecieving();
}
CmdUserList::~CmdUserList(){}
bool CmdUserList::processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.set( getMsgType());
   return true;
}
void CmdUserList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.stdOut( Verbose);
}

CmdUserJobsList::CmdUserJobsList()
{
   setCmd("ujobs");
   setArgsCount(1);
   setInfo("List of user jobs.");
   setHelp("ujobs [id] Request a list of jobs of user with given id.");
   setMsgType( af::Msg::TJobsListRequestUserId);
   setMsgOutType( af::Msg::TJobsList);
   setRecieving();
}
CmdUserJobsList::~CmdUserJobsList(){}
bool CmdUserJobsList::processArguments( int argc, char** argv, af::Msg &msg)
{
   bool ok; int number = (QString::fromUtf8(argv[0])).toInt( &ok);
   if( ok == false ) return false;
   msg.set( getMsgType(), number);
   return true;
}
void CmdUserJobsList::msgOut( af::Msg& msg)
{
   af::MCAfNodes list( &msg);
   list.stdOut( Verbose);
}

CmdUserAdd::CmdUserAdd()
{
   setCmd("uadd");
   setArgsCount(1);
   setInfo("Add permanent user.");
   setHelp("uadd [name] Add a permanent user with spcecified name.");
   setMsgType( af::Msg::TUserAdd);
}
CmdUserAdd::~CmdUserAdd(){}
bool CmdUserAdd::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   af::MCGeneral mcgeneral( name, 1);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdUserDelete::CmdUserDelete()
{
   setCmd("udel");
   setArgsCount(1);
   setInfo("Delete permanent user.");
   setHelp("udel [name] Delete a permanent user with spcecified name.");
   setMsgType( af::Msg::TUserDel);
}
CmdUserDelete::~CmdUserDelete(){}
bool CmdUserDelete::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   af::MCGeneral mcgeneral( name, 0);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdUserPriority::CmdUserPriority()
{
   setCmd("upri");
   setArgsCount(2);
   setInfo("Change render priority.");
   setHelp("upri [name] [priority] Set user priority.");
   setMsgType( af::Msg::TUserPriority);
}
CmdUserPriority::~CmdUserPriority(){}
bool CmdUserPriority::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   bool ok; int number = (QString::fromUtf8(argv[1])).toInt( &ok);
   if( ok == false ) return false;
   af::MCGeneral mcgeneral( name, number);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdUserHostsMaximum::CmdUserHostsMaximum()
{
   setCmd("uhmax");
   setArgsCount(2);
   setInfo("Set user maximum hosts.");
   setHelp("uhmax [name] [number] Change user maximum hosts.");
   setMsgType( af::Msg::TUserMaxHosts);
}
CmdUserHostsMaximum::~CmdUserHostsMaximum(){}
bool CmdUserHostsMaximum::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   bool ok; int number = (QString::fromUtf8(argv[1])).toInt( &ok);
   if( ok == false ) return false;
   af::MCGeneral mcgeneral( name, number);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}

CmdUserHostsMask::CmdUserHostsMask()
{
   setCmd("uhmask");
   setArgsCount(2);
   setInfo("Set user hosts max.");
   setHelp("uhmask [name] [string] Change user hosts mask.");
   setMsgType( af::Msg::TUserHostsMask);
}
CmdUserHostsMask::~CmdUserHostsMask(){}
bool CmdUserHostsMask::processArguments( int argc, char** argv, af::Msg &msg)
{
   QString name = QString::fromUtf8(argv[0]);
   QString mask = QString::fromUtf8(argv[1]);
   QRegExp rx( mask, Qt::CaseInsensitive);
   if( rx.isValid() == false )
   {
      printf("QRegExp: %s\n", rx.errorString().toUtf8().data());
      return false;
   }
   af::MCGeneral mcgeneral( name, mask);
   msg.set( getMsgType(), &mcgeneral);
   return true;
}
