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

/// Set variable capacity
   void setVariableCapacity( int min, int max);

/// Set block tasks to multihost
   void setMultiHost( int min, int max, int waitmax, bool sameHostMaster, const std::string & m_service, int waitsrv);

public:
   inline void setBlockNumber( int value) { m_block_num = value;}
   virtual int calcWeight() const; ///< Calculate and return memory size.
   void fillTasksArrayFromList();

private:
   std::list<Task*> tasks;
   bool deleteTasksWithBlock;
};
}
