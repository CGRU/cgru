#pragma once

#include "../libafanasy/blockdata.h"

#include "api.h"

namespace afapi
{
class Task;

class Block : public af::BlockData
{
public:
   Block();
   ~Block();

/// Append tasks list
   bool appendTask( Task * task);

/// Clear tasks list
   void clearTasksList();

   void stdOut( bool full = false ) const;

/// Set name.
   void setName( const char * str);

/// Set tasks service type name.
   void setTasksType( const char * str);

/// Set tasks name.
   void setTasksName( const char * str);

/// Set tasks parser type ( if parser type name does not match tasks type ).
   void setParserType( const char * str);

/// Set current block tasks working directory.
   void setWorkingDirectory( const char * str);

/// Set block tasks extra environment.
   void setEnvironment( const char * str);

/// Set block command.
   void setCommand( const char * str);

/// Set block tasks view result command.
   void setCommandView( const char * str);

/// Set block command type to numeric. %1 and %2 arguments in command will be replaced for each task.
   bool setNumeric( int start, int end, int perHost = 1, int incement = 1);

/// Set not numeric block frames per host number (for dependicies and parsers)
   void setFramesPerTask( int perTask);

/// Set block pre register command, witch will be executed before job register.
   void setCmdPre(  const char * str);

/// Set block post deletion command, witch will be executed before job register.
   void setCmdPost( const char * str);

/// Set tasks maximum run time, after this time task will be restart as error task
   void setTasksMaxRunTime( int value);

/// Set block maximum hosts.
   void setMaxHosts( int value);

/// Set block hosts mask.
   bool setHostsMask( const char * str);

/// Set block hosts to exclude mask.
   bool setHostsMaskExclude( const char * str);

/// Set block depend mask.
   bool setDependMask( const char * str);

/// Set tasks depend mask.
   bool setTasksDependMask( const char * str);

/// Set tasks capacity.
   bool setCapacity( int value );

/// Set needed properties.
   bool setNeedProperties( const char * str);

/// Set needed memory.
   void setNeedMemory( int value );

/// Set needed power.
   void setNeedPower(  int value );

/// Set needed hdd.
   void setNeedHDD(    int value );

/// Set file parameters to check
   void setFileSizeCheck( int min, int max);

/// Set variable capacity
   void setVariableCapacity( int min, int max);

/// Set block tasks to multihost
   void setMultiHost( int min, int max, int waitmax, bool sameHostMaster, const char * service, int waitsrv);

public:
   inline void setBlockNumber( int value) { blocknum = value;}
   inline void setName( const QString & str) { name = str; }
   virtual int calcWeight() const; ///< Calculate and return memory size.
   void fillTasksArrayFromList();

private:
   std::list<Task*> tasks;
};
}
