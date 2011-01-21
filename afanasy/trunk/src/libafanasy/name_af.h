#pragma once

#include <time.h>

#include <QtCore/qregexp.h>
#include <QtCore/QString>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>

#include "../include/aftypes.h"

namespace af
{
   class Attr;
   class Af;

   class Msg;

   class Environment;

   class Address;
   class AddressesList;
   class Passwd;
   class Farm;
   class Host;
   class HostRes;
   class Parser;
   class PyClass;
   class Service;

   class MsgClass;
   class MsgClassUserHost;
   class MCTest;
   class MCGeneral;
   class MCAfNodes;
   class MCAfNodesIt;
   class MCTalkmessage;
   class MCTalkdistmessage;
   class MCTaskPos;
   class MCTasksPos;
   class MCTasksProgress;
   class MCListenAddress;
   class MCTaskUp;
   class MCJobsWeight;

   class Node;

   class Client;
   class Talk;
   class Render;
   class Monitor;

   class UserHeader;
   class User;

   class JobHeader;
   class JobInfo;
   class Job;
   class BlockData;
   class TaskData;
   class TaskExec;
   class TaskProgress;
   class JobProgress;

   void printTime( uint32_t time32 = 0);

   const QString time2Qstr( uint32_t time32 = time( NULL));
   const QString time2QstrHMS( uint32_t time32, bool clamp = false);
   const QString state2str( int state);
   const QString fillNumbers( const QString & pattern, int start, int end);

   bool setRegExp( QRegExp & regexp, const QString & str, const QString & name);

   void filterFileName( QString & filename);

   void rw_int32 (  int32_t &integer, char * data, bool write);
   void rw_uint32( uint32_t &integer, char * data, bool write);

   int weigh( const QString & str);
   int weigh( const QRegExp & regexp);


   bool  init( uint32_t flags );
   enum InitFlags
   {
      NoFlags      = 0,
      Verbose      = 1,
      InitFarm     = 1 << 1,
   };
   void  destroy();

   bool  loadFarm( bool verbose = false);
   bool  loadFarm( const QString & filename, bool verbose = false);
   const Farm * farm();
}
