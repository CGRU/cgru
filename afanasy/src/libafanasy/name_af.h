#pragma once

#include <algorithm>
#include <iostream>
#include <list>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

#ifndef WINNT
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#include <Python.h>

#include "rapidjson/document.h"
typedef rapidjson::Value JSON;

struct sockaddr_storage;

namespace af
{
   class Attr;
   class Af;
   class AfQueue;
   class AfQueueItem;

   class Msg;
   class MsgQueue;
   class MsgStat;

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

	enum InitFlags
	{
		NoFlags      = 0,
		InitVerbose  = 1,
		InitFarm     = 1 << 1,
	};

   enum Direction
   {
      Left,
      Right
   };

	enum MagickMismatchMode
	{
		MMM_Reject  = 0, ///< Messages with invalid macgick number will be rejected.
		MMM_GetOnly = 1, ///< You can get information only, no change allowed.
		MMM_NoTasks = 2, ///< You can't add jobs and change existing tasks commands.
	};

	void outError( const char * errMsg, const char * baseMsg = NULL);

	void sleep_sec(  int i_seconds  );
	void sleep_msec( int i_mseconds );

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
	const std::string strEscape( const std::string & i_str);
	const std::vector<std::string> strSplit( const std::string & str, const std::string & separators = "|;,: ");
	const std::string vectToStr( const std::vector<int32_t> & i_vec);

	int getReadyTaskNumber( int i_quantity, TaskProgress ** i_tp, int32_t flags, int i_startFrom = 0);
   const std::string fillNumbers( const std::string & pattern, long long start, long long end);
   const std::string replaceArgs( const std::string & pattern, const std::string & arg);

   const std::string time2str( time_t time_sec = time( NULL), const char * time_format = NULL);
   const std::string time2strHMS( int time32, bool clamp = false);

	bool addUniqueToList( std::list<int32_t> & o_list, int i_value);
	bool addUniqueToVect( std::vector<int> & o_vect, int i_value);

	void printTime( time_t time_sec = time( NULL), const char * time_format = NULL);

	void printAddress( struct sockaddr_storage * i_ss );

	bool setRegExp( RegExp & regexp, const std::string & str, const std::string & name, std::string * errOutput = NULL);

   void rw_int32 (  int32_t &integer, char * data, bool write);
   void rw_uint32( uint32_t &integer, char * data, bool write);

   int weigh( const std::string & str);
   int weigh( const std::list<std::string> & strlist);


   bool  init( uint32_t flags );
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
	bool pathMakePath( const std::string & i_path, VerboseMode i_verbose = VerboseOff);
	const std::vector<std::string> getFilesList( const std::string & i_path);

   bool netIsIpAddr( const std::string & addr, bool verbose = false);

   char * fileRead( const std::string & filename, int & readsize, int maxfilesize = -1, std::string * errOutput = NULL);

#ifdef WINNT
    void launchProgram( const std::string & i_commandline, const std::string & i_wdir = std::string());
    bool launchProgram( PROCESS_INFORMATION * o_pinfo,
	                    const std::string & i_commandline, const std::string & i_wdir = std::string(),
                        HANDLE * o_in = NULL, HANDLE * o_out = NULL, HANDLE * o_err = NULL,
                        DWORD i_flags = 0, bool alwaysCreateWindow = false);
#else
    int launchProgram( const std::string & i_commandline, const std::string & i_wdir = std::string(),
                       FILE ** o_in = NULL, FILE ** o_out = NULL, FILE ** o_err = NULL);
#endif

    /// Calculate messages statistics and write it in buffer, return written size.
    void statwrite( af::Msg * msg = NULL );

    /// Read messages statistics from buffer.
    void statread( af::Msg * msg );

    /// Write messages statistics ( in \c stdout ).
    void statout( int  columns = -1, int sorting = -1);

    /// Solve host address.
    const af::Address solveNetName( const std::string & i_name, int i_port, int i_type = AF_UNSPEC, VerboseMode i_verbose = VerboseOff);

	Msg * msgString( const std::string & i_str);

    /// Recieve message from given file discriptor \c desc to \c buffer
    /** Return true if success. This function will block process.**/
    bool msgread( int desc, af::Msg* msg);

    /// Send message \c msg to given file discriptor
    /** Return true if success.**/
    bool msgwrite( int i_desc, const af::Msg * i_msg);

    /// Send a message to all its addresses and receive an answer if needed
    Msg * msgsend( Msg * i_msg, bool & io_ok, VerboseMode i_verbose);

    bool PyGetString( PyObject * obj, std::string & str, const char * errMsg = NULL);

	const std::string jsonMakeHeader( int size);
	char * jsonParseData( rapidjson::Document & o_doc, const char * i_data, int i_data_len, std::string * o_err = NULL);
	char * jsonParseMsg( rapidjson::Document & o_doc, const af::Msg * i_msg, std::string * o_err = NULL);
	bool jr_string( const char * i_name, std::string & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_regexp( const char * i_name, RegExp      & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_bool  ( const char * i_name, bool        & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_int   ( const char * i_name, int         & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_int8  ( const char * i_name, int8_t      & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_uint8 ( const char * i_name, uint8_t     & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_int16 ( const char * i_name, int16_t     & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_uint16( const char * i_name, uint16_t    & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_int32 ( const char * i_name, int32_t     & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_uint32( const char * i_name, uint32_t    & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_int64 ( const char * i_name, int64_t     & o_attr, const JSON & i_object, std::string * o_str = NULL);
	bool jr_int32vec(  const char * i_name, std::vector<int32_t>     & o_attr, const JSON & i_object);
	bool jr_stringvec( const char * i_name, std::vector<std::string> & o_attr, const JSON & i_object);

	void jw_state( uint32_t i_state, std::ostringstream & o_str, bool i_render = false);

	af::Msg * jsonMsg( const std::string & i_str);
	af::Msg * jsonMsgError( const std::string & i_str);
	af::Msg * jsonMsg( const std::ostringstream & i_stream);
	af::Msg * jsonMsg( const std::string & i_type, const std::string & i_name, const std::list<std::string> & i_list);
	af::Msg * jsonMsg( const std::string & i_type, const std::string & i_name, const std::string & i_string);
	af::Msg * jsonMsg( const std::string & i_type, const std::string & i_name, char * i_data, int i_size);

	void jsonActionStart(  std::ostringstream & i_str, const std::string & i_type,
		const std::string & i_mask, const std::vector<int> & i_ids = std::vector<int>());
	void jsonActionFinish( std::ostringstream & i_str);

	void jsonActionParamsStart(  std::ostringstream & i_str, const std::string & i_type,
		const std::string & i_mask, const std::vector<int> & i_ids = std::vector<int>());
	void jsonActionParamsFinish( std::ostringstream & i_str);

	void jsonActionOperation(  std::ostringstream & i_str, const std::string & i_type, const std::string & i_operation,
		const std::string & i_mask, const std::vector<int> & i_ids = std::vector<int>());
	void jsonActionOperationStart(  std::ostringstream & i_str, const std::string & i_type, const std::string & i_operation,
		const std::string & i_mask, const std::vector<int> & i_ids = std::vector<int>());
	void jsonActionOperationFinish( std::ostringstream & i_str);
}
