#ifndef MCTASKSPOS_H
#define MCTASKSPOS_H

#include <aftypes.h>
#include <stdlib.h>
#include <vector>

#include <QtCore/QString>

#include "msgclassuserhost.h"

namespace af
{

class MCTasksPos : public MsgClassUserHost
{
public:
   MCTasksPos( int job_id, const QString &Message);
   MCTasksPos( Msg * msg);
   ~MCTasksPos();

   bool addBlock( int numblock);
   bool addTask(  int numblock, int numtask);

   inline bool hasTasks() const { return has_tasks;}
   void stdOut( bool full = false ) const;


   inline int getJobId() const { return jobid;          }
   inline int getCount() const { return numbloks.size();}
   inline const QString& getMessage() const { return message; }

   inline int getNumBlock( const unsigned pos) const
      { if( pos < numbloks.size()) return numbloks[pos]; else return -1; }
   inline int getNumTask(  const unsigned pos) const
      { if( has_tasks && (pos < numbloks.size())) return numtasks[pos]; else return -1; }

private:
   int32_t jobid;
   bool     has_tasks;
   std::vector<int32_t> numbloks;
   std::vector<int32_t> numtasks;
   QString message;

   void readwrite( Msg * msg);
};
}
#endif
