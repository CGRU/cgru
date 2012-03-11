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
   void setName( const std::string & str);

/// Set task command.
   void setCommand( const std::string & str);

/// Set task view result command.
   void setFiles( const std::string & str);

/// Set task custom data.
   void setCustomData( const std::string & str);

/// Set task depend mask.
   bool setDependMask( const std::string & str);
};
}
