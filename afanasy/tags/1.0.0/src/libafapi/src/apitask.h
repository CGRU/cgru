#pragma once
#include <taskdata.h>

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
   void setCommandView( const char * str);

/// Set task depend mask.
   bool setDepend( const char * str);

/// Set task name.
   inline void setName( const QString & str) { name = str; }
};
}
