#pragma once

#include "../libafanasy/job.h"
#include "../libafanasy/msg.h"

#include "api.h"

namespace afapi
{
class Block;

class Job : public af::Job
{
public:

   Job( bool DeleteBlocksWithJob = true);
   ~Job();

   void setUserName( const std::string & str);
   void setHostName( const std::string & str);

/// Print main job information.
   void output( bool full = false);

/// Set new job name.
   void setName( const std::string & str);

/// Set job description ( for statistics purposes only).
   void setDescription( const std::string & str);

/// Set job pre command ( executed by server on job registration )
   void setCmdPre( const std::string & str);

/// Set job post command ( executed by server on job deletion )
   void setCmdPost( const std::string & str);

/// Set job maximum hosts.
   void setMaxRunningTasks( int value);

/// Set job hosts mask.
/** See http://doc.trolltech.com/main-snapshot/qregexp.html#introduction .**/
   bool setHostsMask( const std::string & str);

/// Set job exclude hosts mask.
   bool setHostsMaskExclude( const std::string & str);

/// Set job depend mask.
   bool setDependMask( const std::string & str);

/// Set job global depend mask ( job will depend on job not from the same user).
   bool setDependMaskGlobal( const std::string & str);

/// Set needed OS.
   bool setNeedOS( const std::string & str);

/// Set needed properties.
   bool setNeedProperties( const std::string & str);

/// Set job priorityhttp://html.manual.ru/book/info/basecolors.phptv.
   void setPriority( int value);

/// Set job start wait time.
   void setWaitTime( long long value);

/// Set job state to offline. Afanasy will not solve it.
   void offline();


/// Clear blocks pointers list
   void clearBlocksList();
/// Append block to job blocks list
   bool appendBlock( Block * block);
/// Construct job and data for message
   bool construct();
/// Get raw job data, to send to server socket ( call after successfull).
   char * getData();
/// Get raw job data length.
   int getDataLen();

public:
   void fillBlocksDataPointersFromList();

private:
   af::Msg * message;
   std::list<Block*> blocks;
   bool deleteBlocksWithJob;

private:
   void setUniqueBlockName( Block * block);
   void deleteBlocksDataPointers();
};
}
