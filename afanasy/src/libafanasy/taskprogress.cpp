#include "taskprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

TaskProgress::TaskProgress()
	: state(0), percent(0), frame(0), percentframe(0), starts_count(0), errors_count(0), time_start(0),
	  time_done(0)
{
}

TaskProgress::TaskProgress(Msg *msg) { read(msg); }

TaskProgress::~TaskProgress() {}

void TaskProgress::v_readwrite(Msg *msg)
{
	rw_int64_t(state, msg);
	rw_int8_t(percent, msg);
	rw_int64_t(frame, msg);
	rw_int8_t(percentframe, msg);
	rw_int32_t(starts_count, msg);
	rw_int32_t(errors_count, msg);
	rw_int64_t(time_start, msg);
	rw_int64_t(time_done, msg);
	rw_String(hostname, msg);
	rw_String(activity, msg);
	rw_String(resources, msg);
}

void TaskProgress::jsonRead(const JSON &i_obj)
{
	jr_int64("st", state, i_obj);
	jr_int32("str", starts_count, i_obj);
	jr_int32("err", errors_count, i_obj);
	jr_int64("tst", time_start, i_obj);
	jr_int64("tdn", time_done, i_obj);
	jr_string("hst", hostname, i_obj);
	jr_string("res", resources, i_obj);
}

void TaskProgress::jsonWrite(std::ostringstream &o_str) const
{
	o_str << "{";
	jw_stateJob(state, o_str);
	o_str << ",\"st\":" << state;
	if (percent > 0)
		o_str << ",\"per\":" << int(percent);
	if (frame > 0)
		o_str << ",\"frm\":" << frame;
	if (percentframe > 0)
		o_str << ",\"pfr\":" << int(percentframe);
	if (starts_count > 0)
		o_str << ",\"str\":" << starts_count;
	if (errors_count > 0)
		o_str << ",\"err\":" << errors_count;
	if (time_start > 0)
		o_str << ",\"tst\":" << time_start;
	if (time_done > 0)
		o_str << ",\"tdn\":" << time_done;
	if (hostname.size())
		o_str << ",\"hst\":\"" << hostname << "\"";
	if (activity.size())
		o_str << ",\"act\":\"" << activity << "\"";
	if (resources.size())
		o_str << ",\"res\":\"" << resources << "\"";
	//	int no_progress_for = last_progress_change - time(NULL);
	//	if (no_progress_for > 0) o_str << ",\"npf\":" << no_progress_for;
	o_str << "}";
}

int TaskProgress::calcWeight() const
{
	int weight = sizeof(TaskProgress);
	weight += weigh(hostname);
	return weight;
}

void TaskProgress::v_generateInfoStream(std::ostringstream &stream, bool full) const
{
	static const char time_format[] = "%H:%M.%S";
	stream << "s" << state;
	stream << " p" << int(percent) << "%";
	stream << " (" << frame << "-" << int(percentframe) << "%)";
	stream << "s" << starts_count << "/" << starts_count << "e";
	stream << " (" << af::time2str(time_start, time_format);
	stream << "-" << af::time2str(time_done, time_format);
	stream << "=" << af::time2str(time_done - time_start, time_format) << ")";
	//	int no_progress_for = last_progress_change - time(NULL);
	//	if( no_progress_for > 0 ) stream << " npf" << no_progress_for;
	if (false == hostname.empty())
		stream << " - " << hostname;
}
