#pragma once

#include <stdlib.h>
#include <vector>

#include "af.h"
#include "taskexec.h"

namespace af
{

/**
 * @brief The RenderReconnect class is holding the render ID and the full list
 * of the task that the render is currently running.
 * It is used when trying to reconnect to a server, after the later restarted
 * (potentially because it crashed)
 */
class RenderReconnect : public Af
{
public:
	RenderReconnect();
	RenderReconnect( Msg * msg);

	~RenderReconnect();

	inline void setId( int32_t i_id) { m_id = i_id; }
	inline int32_t getId() const { return m_id; }

	void addTaskExec( TaskExec * i_task);
	std::vector<TaskExec*> &getTaskExecs() { return m_taskexecs; }

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

public:
	void clear();

private:
	int32_t m_id;

	// Note: Do not delete m_taskexecs[i] since taskexecs are owned by the
	// render host
	std::vector<TaskExec*> m_taskexecs;
	bool m_server_side;

private:
	void v_readwrite( Msg * msg);
};
}
