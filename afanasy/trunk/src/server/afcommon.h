#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "../libafanasy/name_af.h"

#include "../libafsql/name_afsql.h"

#include "msgqueue.h"
#include "filequeue.h"
#include "dbupdatetaskqueue.h"
#include "dbactionqueue.h"
#include "commandsqueue.h"

class AFCommon
{
public:
   AFCommon();
   ~AFCommon();

   static bool createDirectory( const char * name, bool verbose = false);
   static void executeCmd( const QString & cmd);             ///< Execute command.

/// Save string list, perform log file rotation;
   static void saveLog( const QStringList & log, const QString & dirname, QString filename, int rotate);

   static bool writeFile( const char * data, const int length, const char * name); ///< Write a file

//   static void catchDetached(); ///< Try to wait any child ( to prevent Zombie processes).

   inline static void MsgDispatchQueueRun() { if( MsgDispatchQueue) MsgDispatchQueue->run(); }
   inline static void FileWriteQueueRun()   { if( FileWriteQueue)   FileWriteQueue->run();   }
   inline static void CmdExecQueueRun()     { if( CmdExecQueue)     CmdExecQueue->run();     }
   inline static void DBUpTaskQueueRun()    { if( DBUpTaskQueue)    DBUpTaskQueue->run();    }
   inline static void DBUpdateQueueRun()    { if( DBUpdateQueue)    DBUpdateQueue->run();    }

   inline static bool QueueMsgDispatch( MsgAf* msg)       { if( MsgDispatchQueue ) return MsgDispatchQueue->pushMsg( msg);     else return false;}
   inline static bool QueueFileWrite( FileData* filedata) { if( FileWriteQueue   ) return FileWriteQueue->pushFile( filedata); else return false;}
   inline static bool QueueCmdExec( const QString & cmd)  { if( CmdExecQueue     ) return CmdExecQueue->pushCommand( cmd);     else return false;}

   inline static void QueueDBAddItem(    const afsql::DBItem * item) { if( DBUpdateQueue ) DBUpdateQueue->addItem(    item );}
   inline static void QueueDBDelItem(    const afsql::DBItem * item) { if( DBUpdateQueue ) DBUpdateQueue->delItem(    item );}
   inline static void QueueDBUpdateItem( const afsql::DBItem * item, int attr = -1) { if( DBUpdateQueue ) DBUpdateQueue->updateItem( item, attr );}

   inline static void QueueDBUpdateTask_begin() { DBUpTaskQueue->lock(); }
   inline static bool QueueDBUpdateTask(  int JobId, int BlockNum, int TaskNum, const af::TaskProgress * Progress)
                         { return DBUpTaskQueue->pushTaskUp( JobId, BlockNum, TaskNum, Progress);}
   inline static void QueueDBUpdateTask_end() { DBUpTaskQueue->unlock(); }

   inline static void MsgDispatchQueueQuit() { if( MsgDispatchQueue) MsgDispatchQueue->quit(); }
   inline static void FileWriteQueueQuit()   { if( FileWriteQueue)   FileWriteQueue->quit();   }
   inline static void CmdExecQueueQuit()     { if( CmdExecQueue)     CmdExecQueue->quit();     }
   inline static void DBUpTaskQueueQuit()    { if( DBUpTaskQueue)    DBUpTaskQueue->quit();    }
   inline static void DBUpdateQueueQuit()    { if( DBUpdateQueue)    DBUpdateQueue->quit();    }

private:
   static MsgQueue          * MsgDispatchQueue;
   static FileQueue         * FileWriteQueue;
   static CommandsQueue     * CmdExecQueue;
   static DBUpdateTaskQueue * DBUpTaskQueue;
   static DBActionQueue     * DBUpdateQueue;

//   static bool detach();
};
