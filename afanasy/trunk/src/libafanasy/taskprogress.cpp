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
   rw_QString ( hostname,     msg);
}

int TaskProgress::calcWeight() const
{
   int weight = sizeof(TaskProgress);
   weight += weigh( hostname);
   return weight;
}

void TaskProgress::stdOut( bool full ) const
{
   QString timeformat("hh:mm.ss");
   printf("s%d p%d%% (%d-%d%%) s%d/%de (%s-%s=%s) - %s\n", state, percent, frame, percentframe, starts_count, errors_count,
      QDateTime::fromTime_t(  time_start ).toString( timeformat).toUtf8().data(),
      QDateTime::fromTime_t(  time_done  ).toString( timeformat).toUtf8().data(),
      time2QstrHMS( time_done - time_start ).toUtf8().data(),
      hostname.toUtf8().data());
}
