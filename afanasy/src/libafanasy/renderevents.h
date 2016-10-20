#pragma once

#include <stdlib.h>
#include <vector>

#include "af.h"
#include "taskexec.h"
#include "msgclasses/mctaskpos.h"

namespace af
{

class RenderEvents : public Af
{
public:
	RenderEvents();
	RenderEvents( Msg * msg);

	~RenderEvents();

	inline void addTaskExec( TaskExec * i_exec ) { m_tasks.push_back( i_exec);}
	void remTaskExec( const TaskExec * i_exec );
	inline void clearTaskExecs() { m_tasks.clear();}

	inline void addTaskClose(  const MCTaskPos & i_tp) { addUniqueTask( i_tp, m_closes  );}
	inline void addTaskStop(   const MCTaskPos & i_tp) { addUniqueTask( i_tp, m_stops   );}
	inline void addTaskOutput( const MCTaskPos & i_tp) { addUniqueTask( i_tp, m_outputs );}
	inline void taskListenAdd( const MCTaskPos & i_tp) { addUniqueTask( i_tp, m_listens_add );}
	inline void taskListenRem( const MCTaskPos & i_tp) { addUniqueTask( i_tp, m_listens_rem );}

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

public:

	// This is job solving tasks.
	std::vector<TaskExec*> m_tasks;

	// Tasks to close:
	std::vector<MCTaskPos> m_closes;

	// Tasks to stop:
	std::vector<MCTaskPos> m_stops;

	// Tasks outputs:
	std::vector<MCTaskPos> m_outputs;

	// Tasks listens:
	std::vector<MCTaskPos> m_listens_add;
	std::vector<MCTaskPos> m_listens_rem;

	// Exit, reboot, sleep and so on.
	std::string m_instruction;

	// Command to execute.
	// This not job solving (not a task), for example to sleep
	std::string m_command;

public:

	bool isEmpty() const;

	void clear();

private:
	void addUniqueTask( const MCTaskPos & i_tp, std::vector<MCTaskPos> & o_vec);

	void rw_tp_vec( std::vector<MCTaskPos> & io_vec, Msg * io_msg);
	void rw_texecs( std::vector<TaskExec*> & io_vec, Msg * io_msg);

	void v_readwrite( Msg * msg);
};
}
