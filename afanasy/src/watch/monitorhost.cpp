#include "monitorhost.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

MonitorHost * MonitorHost::m_ = NULL;

int MonitorHost::ms_uid = -1;

std::vector<int32_t> MonitorHost::ms_ids;

std::map<std::string, int> MonitorHost::ms_subsribes_counts;

MonitorHost::MonitorHost()
{
	m_ = this;
	ms_ids.push_back(0);
}

MonitorHost::~MonitorHost()
{
}

af::Msg * MonitorHost::genRegisterMsg()
{
	std::ostringstream str;

	str << "{\"monitor\":{";
	str << "\"binary\":true";
	str << ",\"user_name\":\"" << af::Environment::getUserName() << "\"";
	str << ",\"host_name\":\"" << af::Environment::getHostName() << "\"";
	str << ",\"engine\":\"" << af::Environment::getVersionCGRU() << "\"";
	str << "}}";

	af::Msg * msg = af::jsonMsg( str);

	return msg;
}

void MonitorHost::connectionLost()
{
	m_->m_id  =  0;
	m_->m_uid =  0;
	ms_uid    = -1;
	ms_ids[0] =  0;
}

void MonitorHost::connectionEstablished( int i_id, int i_uid)
{
	m_->m_id = i_id;
	m_->m_uid = i_uid;
	ms_uid = i_uid;
	ms_ids[0] = i_id;
}

void MonitorHost::subscribe( const std::string & i_class, bool i_subscribe)
{
	std::map<std::string, int>::iterator it = ms_subsribes_counts.find(i_class);
	if (it == ms_subsribes_counts.end())
		ms_subsribes_counts[i_class] = 0;

	if (i_subscribe)
	{
		ms_subsribes_counts[i_class]++;

		if (ms_subsribes_counts[i_class] > 1)
			return;
	}
	else
	{
		ms_subsribes_counts[i_class]--;

		if (ms_subsribes_counts[i_class] < 0)
		{
			ms_subsribes_counts[i_class] = 0;
			AF_ERR << "Was not subscibed on '" << i_class << "'";
			return;
		}

		if (ms_subsribes_counts[i_class] > 0)
			return;
	}

	std::vector<int> ids;
	ids.push_back( m_->getId());

	std::ostringstream str;
	af::jsonActionOperationStart( str,"monitors","watch", std::string(), ids);
	str << ",\"class\":\"" << i_class << "\"";
	str << ",\"status\":\"" << ( i_subscribe ? "subscribe":"unsubscribe") << "\"";
	af::jsonActionOperationFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

void MonitorHost::setJobId( int i_jid, bool i_add)
{
AFINFA("MonitorHost::setJobId: jid=%d, add=%d", i_jid, i_add);

	std::vector<int> ids;
	ids.push_back( m_->getId());

	std::ostringstream str;
	af::jsonActionOperationStart( str,"monitors","watch","", ids);
	str << ",\"class\":\"tasks\"";
	str << ",\"status\":\"" << ( i_add ? "subscribe" : "unsubscribe" ) << "\"";
	str << ",\"ids\":[" << i_jid << "]";
	af::jsonActionOperationFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

void MonitorHost::setUid( int i_uid)
{
AFINFO("MonitorHost::setUid:");

	// If it is first time, we store it;
	if( ms_uid < 0 )
		ms_uid = i_uid;

	// Negative value means restore original:
	if( i_uid < 0 )
		i_uid = m_->m_uid;

	std::vector<int> ids;
	ids.push_back( m_->getId());

	std::ostringstream str;
	af::jsonActionOperationStart( str,"monitors","watch","", ids);
	str << ",\"class\":\"perm\"";
	str << ",\"uid\":" << i_uid;
	af::jsonActionOperationFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

