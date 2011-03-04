#include "taskprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

TaskProgress::TaskProgress():
   state(0),
   percent(0),
   frame(0),
   percentframe(0),
   starts_count(0),
   errors_count(0),
   time_start(0),
   time_done(0)
{
}

TaskProgress::TaskProgress( Msg * msg)
{
   read( msg);
}

TaskProgress::~TaskProgress()
{
}

void TaskProgress::readwrite( Msg * msg)
{
   rw_uint32_t( state,        msg);
   rw_int8_t  ( percent,      msg);
   rw_int32_t ( frame,        msg);
   rw_int8_t  ( percentframe, msg);
   rw_int32_t ( starts_count, msg);
   rw_int32_t ( errors_count, msg);
   rw_uint32_t( time_start,   msg);
   rw_uint32_t( time_done,    msg);
   rw_String  ( hostname,     msg);
}

int TaskProgress::calcWeight() const
{
   int weight = sizeof(TaskProgress);
   weight += weigh( hostname);
   return weight;
}

void TaskProgress::generateInfoStream( std::ostringstream & stream, bool full ) const
{
   static const char time_format[] = "%H:%M.%S";
   stream << "s" << state;
   stream << " p" << int(percent) << "%";
   stream << " (" << frame << "-" << int(percentframe) << "%)";
   stream << "s" << starts_count << "/" << starts_count << "e";
   stream << " (" << af::time2str( time_start, time_format);
   stream << "-" << af::time2str( time_done, time_format);
   stream << "=" << af::time2str( time_done - time_start, time_format) << ")";
   if( false == hostname.empty()) stream << " - " << hostname;
}
