#pragma once

#include "../libafanasy/environment.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/name_af.h"

#include "../libafsql/name_afsql.h"

#include "dbqueue.h"
#include "filequeue.h"
#include "logqueue.h"
#include "store.h"

struct ThreadArgs;

/*
	From what I understand this is just a holder for global
	variables (aghiles).
*/
class AFCommon
{
public:
	AFCommon( ThreadArgs * i_threadArgs);
	~AFCommon();

	static void executeCmd( const std::string & cmd); ///< Execute command.

/// Save string list, perform log file rotation;
	static void saveLog( const std::list<std::string> & log, const std::string & dirname, const std::string & filename);

	static bool writeFile( const char * data, const int length, const std::string & filename); ///< Write a file
	inline static bool writeFile( const std::string & i_str, const std::string & i_file_name)
		{ return writeFile( i_str.c_str(), i_str.size(), i_file_name);}
	inline static bool writeFile( const std::ostringstream & i_str, const std::string & i_file_name)
		{ std::string str = i_str.str(); return writeFile( str.c_str(), str.size(), i_file_name);}

	//
	// Store folders:
	//
	static const std::string getStoreDir( const std::string & i_root, int i_id, const std::string & i_name);

	inline static const std::string getStoreDir( const std::string & i_root, const af::Node & i_node)
		{ return getStoreDir( i_root, i_node.getId(), i_node.getName());}

	inline static const std::string getStoreDirJob( const af::Node & i_node)
		{ return getStoreDir( af::Environment::getStoreFolderJobs(), i_node);}

	inline static const std::string getStoreDirRender( const af::Node & i_node)
		{ return getStoreDir( af::Environment::getStoreFolderRenders(), i_node);}

	inline static const std::string getStoreDirUser( const af::Node & i_node)
		{ return getStoreDir( af::Environment::getStoreFolderUsers(), i_node);}

	static const std::vector<std::string> getStoredFolders( const std::string & i_root);

	//
	// Store operations:
	//
	static void saveStore() { return ms_store->save(); }

	static int64_t getJobSerial() { return ms_store->getJobSerial(); }


//   static void catchDetached(); ///< Try to wait any child ( to prevent Zombie processes).

	inline static void QueueFileWrite( FileData * i_filedata)      { FileWriteQueue->pushFile( i_filedata); }
	inline static void QueueNodeCleanUp( const AfNodeSrv * i_node) { FileWriteQueue->pushNode( i_node);     }

	inline static void QueueLog(      const std::string & log) { OutputLogQueue->pushLog( log, LogData::Info  );}
	inline static void QueueLogError( const std::string & log) { OutputLogQueue->pushLog( log, LogData::Error );}
	inline static void QueueLogErrno( const std::string & log) { OutputLogQueue->pushLog( log, LogData::Errno );}

	inline static void DBAddJob( const af::Job * i_job) { if( ms_DBQueue ) ms_DBQueue->addJob( i_job );}
	inline static void DBAddTask(
		const af::TaskExec * i_exec,
		const af::TaskProgress * i_progress,
		const af::Job * i_job,
		const af::Render * i_render)
		{ if( ms_DBQueue ) ms_DBQueue->addTask( i_exec, i_progress, i_job, i_render );}

private:
	static FileQueue * FileWriteQueue;
	static LogQueue  * OutputLogQueue;
	static DBQueue   * ms_DBQueue;

	static Store * ms_store;

//   static bool detach();
};
