#pragma once

#include "../libafanasy/taskdata.h"

#include "api.h"

namespace afapi
{
class Task : public af::TaskData
{
public:
   Task();
   ~Task();

   void stdOut( bool full = false ) const;

/// Set task name.
   void setName( const char * str);

/// Set task command.
   void setCommand( const char * str);

/// Set task view result command.
   void setFiles( const char * str);

/// Set task custom data.
   void setCustomData( const char * str);

/// Set task depend mask.
   bool setDependMask( const char * str);

/// Set task name.
   inline void setName( const std::string & str) { name = str; }
};
}
