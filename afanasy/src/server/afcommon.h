#pragma once

#include "../libafanasy/environment.h"
#include "../libafanasy/msgqueue.h"
#include "../libafanasy/name_af.h"

#include "../libafsql/name_afsql.h"

#include "filequeue.h"
#include "dbupdatetaskqueue.h"
#include "dbactionqueue.h"
#include "cleanupqueue.h"
#include "logqueue.h"

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

	inline static bool writeFile( const std::ostringstream & i_str, const std::string & i_file_name)
		{ std::string str = i_str.str(); writeFile( str.c_str(), str.size(), i_file_name);}
   static bool writeFile( const char * data, const int length, const std::string & filename); ///< Write a file

	static const std::string getStoreDir( const std::string & i_root, int i_id, const std::string & i_name);
	inline static const std::string getStoreDir( const std::string & i_root, const af::Node & i_node)
		{ return getStoreDir( i_root, i_node.getId(), i_node.getName());}
	inline static const std::string getStoreDirJob( const af::Node & i_node)
		{ return getStoreDir( af::Environment::getJobsDir(), i_node);}
	inline static const std::string getStoreDirUser( const af::Node & i_node)
		{ return getStoreDir( af::Environment::getUsersDir(), i_node);}
	inline static const std::string getStoreDirRender( const af::Node & i_node)
		{ return getStoreDir( af::Environment::getRendersDir(), i_node);}

	static const std::vector<std::string> getStoredFolders( const std::string & i_root);

//   static void catchDetached(); ///< Try to wait any child ( to prevent Zombie processes).

	inline static void QueueMsgDispatch( af::Msg * i_msg)          { MsgDispatchQueue->pushMsg( i_msg);     }
	inline static void QueueFileWrite( FileData * i_filedata)      { FileWriteQueue->pushFile( i_filedata); }
	inline static void QueueNodeCleanUp( const AfNodeSrv * i_node) { RemFoldersQueue->pushNode( i_node);    }

   inline static bool QueueLog(        const std::string & log) { if( OutputLogQueue) return OutputLogQueue->pushLog( log, LogData::Info  ); else return false;}
   inline static bool QueueLogError(   const std::string & log) { if( OutputLogQueue) return OutputLogQueue->pushLog( log, LogData::Error ); else return false;}
   inline static bool QueueLogErrno(   const std::string & log) { if( OutputLogQueue) return OutputLogQueue->pushLog( log, LogData::Errno ); else return false;}

   inline static void QueueDBAddItem(    const afsql::DBItem * item) { if( DBUpdateQueue ) DBUpdateQueue->addItem(    item );}
//   inline static void QueueDBDelItem(    const afsql::DBItem * item) { if( DBUpdateQueue ) DBUpdateQueue->delItem(    item );}
//   inline static void QueueDBUpdateItem( const afsql::DBItem * item, int attr = -1) { if( DBUpdateQueue ) DBUpdateQueue->updateItem( item, attr );}

   inline static void QueueDBUpdateTask_begin() { DBUpTaskQueue->lock(); }
   inline static bool QueueDBUpdateTask(  int JobId, int BlockNum, int TaskNum, const af::TaskProgress * Progress)
                         { return DBUpTaskQueue->pushTaskUp( JobId, BlockNum, TaskNum, Progress);}
   inline static void QueueDBUpdateTask_end() { DBUpTaskQueue->unlock(); }

private:
   static af::MsgQueue      * MsgDispatchQueue;
   static FileQueue         * FileWriteQueue;
   static CleanUpQueue      * RemFoldersQueue;
   static LogQueue          * OutputLogQueue;
   static DBUpdateTaskQueue * DBUpTaskQueue;
   static DBActionQueue     * DBUpdateQueue;

//   static bool detach();
};
