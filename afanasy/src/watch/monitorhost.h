#pragma once

#include "../libafanasy/monitor.h"
#include "../libafanasy/msg.h"

#include <QtCore/QList>

class MonitorHost: public af::Monitor
{
public:
	MonitorHost();
	~MonitorHost();

	static af::Msg * genRegisterMsg();

	static inline int id() { return m_->getId();}

	// Some operations needed an array of ids.
	// This function simple returns an array with one id.
	static inline const std::vector<int32_t> & ids() { return ms_ids;}

	static void subscribe( const std::string & i_class, bool i_subscribe);

	static const af::Address & getClientAddress() { return m_->getAddress();}

	static inline void addJobId( int i_jid ) { setJobId( i_jid, true );}
	static inline void delJobId( int i_jid ) { setJobId( i_jid, false);}

	static void setUid( int i_uid);
	static int getUid() { return ms_uid ;}

	static void connectionLost();
	static void connectionEstablished( int i_id, int i_uid);

private:
	static MonitorHost * m_;

	static int ms_uid;

	static std::vector<int32_t> ms_ids;

private:
	static void setJobId( int i_jid, bool i_add);
};
