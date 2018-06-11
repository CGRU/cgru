#pragma once

#include <stdlib.h>
#include <vector>

#include "af.h"
#include "afqueue.h"
#include "host.h"
#include "msgclasses/mctaskpos.h"
#include "msgclasses/mctaskup.h"

namespace af
{

class RenderUpdate : public Af, public AfQueueItem
{
public:
	RenderUpdate();
	RenderUpdate( Msg * msg);

	~RenderUpdate();

	inline void setId( int32_t i_id) { m_id = i_id; }
	inline int32_t getId() const { return m_id; }

	void addTaskUp( MCTaskUp * i_tup);

	void addTaskOutput( const MCTaskPos & i_tp, const std::string & i_output);

	inline void setResources( HostRes * i_hres) { m_hres = i_hres; m_has_hres = true; }
	inline bool hasResources() const { return m_has_hres; }
	inline const HostRes * getResources() const { return m_hres; }

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

public:
	// Tasks to update:.
	std::vector<MCTaskUp*> m_taskups;

	std::vector<std::string> m_outputs;
	std::vector<MCTaskPos> m_outspos;

public:
	void clear();

private:
	int32_t m_id;

	bool m_has_hres;
	HostRes * m_hres;

	bool m_server_side;

private:
	void v_readwrite( Msg * msg);
};

// Needed on server to push tasks updates to run thread:
class RenderUpdatetQueue: public af::AfQueue
{
public:
	RenderUpdatetQueue( std::string i_name): af::AfQueue( i_name, e_no_thread) {}
	~RenderUpdatetQueue() {}
	inline void pushUp( RenderUpdate * i_up) { push( i_up);}
	inline RenderUpdate * popUp( WaitMode i_block ) { return (RenderUpdate*)(pop( i_block));}
};

}
