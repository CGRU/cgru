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

	void addTaskClose( const MCTaskPos & i_tp);
	void addTaskStop(  const MCTaskPos & i_tp);

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

public:

	// This is job solving tasks.
	std::vector<TaskExec*> m_tasks;

	// Tasks to close:
	std::vector<MCTaskPos> m_closes;

	// Tasks to stop:
	std::vector<MCTaskPos> m_stops;

	// Exit, reboot, sleep and so on.
	std::string m_instruction;

	// Command to execute.
	// This not job solving (not a task)
	std::string m_command;

public:

	bool isEmpty() const;

	void clear();

private:
	void v_readwrite( Msg * msg);
};
}
