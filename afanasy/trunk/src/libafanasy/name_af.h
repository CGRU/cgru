#pragma once

#include <algorithm>
#include <iostream>
#include <list>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#include <Python.h>

#include "../include/aftypes.h"

struct sockaddr_storage;

namespace af
{
   class Attr;
   class Af;

   class Msg;

   class Environment;

   class RegExp;
   class NetIF;
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

   enum VerboseMode
   {
      VerboseOff,
      VerboseOn
   };

   enum Direction
   {
      Left,
      Right
   };

   void outError( const char * errMsg, const char * baseMsg = NULL);

   const long long stoi( const std::string & str, bool * ok = NULL);
   const std::string itos( long long integer);
   const std::string getenv( const char * name);
   const std::string state2str( int state);
   const std::string strStrip( const std::string & i_str, const std::string & i_characters = " \n");
   const std::string strStripLeft( const std::string & i_str, const std::string & i_characters = " \n");
   const std::string strStripRight( const std::string & i_str, const std::string & i_characters = " \n");
   const std::string strStrip( const std::string & i_str, Direction i_dir, const std::string & i_characters = " \n");
   const std::string strJoin( const std::list<std::string> & strlist, const std::string & separator = " ");
   const std::string strJoin( const std::vector<std::string> & strvect, const std::string & separator = " ");
   const std::string strReplace( const std::string & str, char before, char after);
   const std::list<std::string> strSplit( const std::string & str, const std::string & separators = "|;,: ");

   const std::string fillNumbers( const std::string & pattern, long long start, long long end);
   const std::string replaceArgs( const std::string & pattern, const std::string & arg);

   const std::string time2str( time_t time_sec = time( NULL), const char * time_format = NULL);
   const std::string time2strHMS( int time32, bool clamp = false);

   void printTime( time_t time_sec = time( NULL), const char * time_format = NULL);
#ifndef WINNT
   void printAddress( struct sockaddr_storage * i_ss );
#endif
   bool setRegExp( RegExp & regexp, const std::string & str, const std::string & name, std::string * errOutput = NULL);

   void rw_int32 (  int32_t &integer, char * data, bool write);
   void rw_uint32( uint32_t &integer, char * data, bool write);

   int weigh( const std::string & str);
   int weigh( const std::list<std::string> & strlist);


   bool  init( uint32_t flags );
   enum InitFlags
   {
      NoFlags      = 0,
      Verbose      = 1,
      InitFarm     = 1 << 1,
   };
   void  destroy();

   bool  loadFarm( bool verbose = false);
   bool  loadFarm( const std::string & filename, bool verbose = false);
   Farm * farm();

   void pathFilterFileName( std::string & filename);
   void pathFilter( std::string & path);
   bool pathIsAbsolute( const std::string & path);
   bool pathFileExists( const std::string & path);
   bool pathIsFolder( const std::string & path);
   const std::string pathAbsolute( const std::string & path);
   const std::string pathUp( const std::string & path);
   const std::string pathHome();
   bool pathMakeDir( const std::string & i_path, VerboseMode i_verbose = VerboseOff);

   bool netIsIpAddr( const std::string & addr, bool verbose = false);

   char * fileRead( const std::string & filename, int & readsize, int maxfilesize = -1, std::string * errOutput = NULL);


   bool PyGetString( PyObject * obj, std::string & str, const char * errMsg = NULL);
}
