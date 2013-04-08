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

void TaskProgress::v_readwrite( Msg * msg)
{
   rw_uint32_t( state,        msg);
   rw_int8_t  ( percent,      msg);
   rw_int64_t ( frame,        msg);
   rw_int8_t  ( percentframe, msg);
   rw_int32_t ( starts_count, msg);
   rw_int32_t ( errors_count, msg);
   rw_int64_t ( time_start,   msg);
   rw_int64_t ( time_done,    msg);
   rw_String  ( hostname,     msg);
	rw_String  ( activity,     msg);
}

void TaskProgress::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "{";
	jw_state( state, o_str);
	if( percent      > 0 ) o_str << ",\"per\":" << int(percent);
	if( frame        > 0 ) o_str << ",\"frm\":" << frame;
	if( percentframe > 0 ) o_str << ",\"pfr\":" << int(percentframe);
	if( starts_count > 0 ) o_str << ",\"str\":" << starts_count;
	if( errors_count > 0 ) o_str << ",\"err\":" << errors_count;
	if( time_start   > 0 ) o_str << ",\"tst\":" << time_start;
	if( time_done    > 0 ) o_str << ",\"tdn\":" << time_done;
	if( hostname.size()  ) o_str << ",\"hst\":\"" << hostname << "\"";
	if( activity.size()  ) o_str << ",\"act\":\"" << activity << "\"";
	o_str << "}";
}

int TaskProgress::calcWeight() const
{
   int weight = sizeof(TaskProgress);
   weight += weigh( hostname);
   return weight;
}

void TaskProgress::v_generateInfoStream( std::ostringstream & stream, bool full ) const
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
