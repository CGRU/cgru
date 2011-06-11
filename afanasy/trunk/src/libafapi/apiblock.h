#pragma once

#include "../libafanasy/blockdata.h"

#include "api.h"

namespace afapi
{
class Task;

class Block : public af::BlockData
{
public:
   Block( bool DeleteTasksWithBlock = true);
   ~Block();

/// Append tasks list
   bool appendTask( Task * task);

/// Clear tasks list
   void clearTasksList();

   void stdOut( bool full = false ) const;

/// Set name.
   void setName( const std::string & str);

/// Set tasks service type name.
   void setService( const std::string & str);

/// Set tasks name.
   void setTasksName( const std::string & str);

/// Set tasks parser type ( if parser type name does not match tasks type ).
   void setParser( const std::string & str);

/// Set current block tasks working directory.
   void setWorkingDirectory( const std::string & str);

/// Set block tasks extra environment.
   void setEnvironment( const std::string & str);

/// Set block command.
   void setCommand( const std::string & str);

/// Set block tasks view result command.
   void setFiles( const std::string & str);

/// Set block command type to numeric.
   bool setNumeric( long long start, long long end, long long perHost = 1, long long incement = 1);

/// Set not numeric block frames per host number (for dependicies and parsers)
   void setFramesPerTask( long long perTask);

/// Set block pre register command, witch will be executed before job register.
   void setCmdPre(  const std::string & str);

/// Set block post deletion command, witch will be executed before job register.
   void setCmdPost( const std::string & str);

/// Set tasks maximum run time, after this time task will be restart as error task
   void setTasksMaxRunTime( int value);

/// Set block hosts mask.
   bool setHostsMask( const std::string & str);

/// Set block hosts to exclude mask.
   bool setHostsMaskExclude( const std::string & str);

/// Set block depend mask.
   bool setDependMask( const std::string & str);

/// Set tasks depend mask.
   bool setTasksDependMask( const std::string & str);

/// Set sub task depend mask.
   bool setSubTaskDependMask( const std::string & str);

/// Set tasks capacity.
   bool setCapacity( int value );

/// Set needed properties.
   bool setNeedProperties( const std::string & str);

/// Set needed memory.
   void setNeedMemory( int value );

/// Set needed power.
   void setNeedPower(  int value );

/// Set needed hdd.
   void setNeedHDD(    int value );

/// Set parser koefficient.
   void setParserCoeff( int value );

/// Set file parameters to check
   void setFileSizeCheck( long long min, long long max);

/// Set variable capacity
   void setVariableCapacity( int min, int max);

/// Set block tasks to multihost
   void setMultiHost( int min, int max, int waitmax, bool sameHostMaster, const std::string & service, int waitsrv);

/// Set task custom data.
   void setCustomData( const std::string & str);

public:
   inline void setBlockNumber( int value) { blocknum = value;}
   virtual int calcWeight() const; ///< Calculate and return memory size.
   void fillTasksArrayFromList();

private:
   std::list<Task*> tasks;
   bool deleteTasksWithBlock;
};
}
