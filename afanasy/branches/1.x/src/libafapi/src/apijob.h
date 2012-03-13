#pragma once

#include <job.h>
#include <msg.h>

#include "api.h"

namespace afapi
{
class Block;

class Job : public af::Job
{
public:

   Job();
   ~Job();

   void setUserName( const char * str);
   void setHostName( const char * str);

/// Print main job information.
   void output( bool full = false);

/// Set new job name.
   void setName( const char* str);

/// Set job description ( for statistics purposes only).
   void setDescription( const char* str);

/// Set job pre command ( executed by server on job registration )
   void setCmdPre( const char * str);

/// Set job post command ( executed by server on job deletion )
   void setCmdPost( const char * str);

/// Set job maximum hosts.
   void setMaxHosts( int value);

/// Set job hosts mask.
/** See http://doc.trolltech.com/main-snapshot/qregexp.html#introduction .**/
   bool setHostsMask( const char* str);

/// Set job exclude hosts mask.
   bool setHostsMaskExclude( const char* str);

/// Set job depend mask.
   bool setDependMask( const char* str);

/// Set job global depend mask ( job will depend on job not from the same user).
   bool setDependMaskGlobal( const char* str);

/// Set needed OS.
   bool setNeedOS( const char* str);

/// Set needed properties.
   bool setNeedProperties( const char* str);

/// Set job priorityhttp://html.manual.ru/book/info/basecolors.phptv.
   void setPriority( int value);

/// Set job start wait time.
   void setWaitTime( int value);

/// Set job state to offline. Afanasy will not solve it.
   void offLine();


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

private:
   af::Msg * message;
   std::list<Block*> blocks;

private:
   void setUniqueBlockName( Block * block);
   void deleteBlocksDataPointers();
   void fillBlocksDataPointersFromList();
};
}
