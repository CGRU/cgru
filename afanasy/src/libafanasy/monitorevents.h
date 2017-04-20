#pragma once

#include <stdlib.h>
#include <vector>

#include "msgclasses/mctask.h"
#include "msgclasses/mctaskpos.h"
#include "taskprogress.h"

namespace af
{

class MonitorEvents : public Af
{
public:
	MonitorEvents();
	MonitorEvents( Msg * msg);

	virtual ~MonitorEvents();

	void v_generateInfoStream( std::ostringstream & o_str, bool i_full = false) const;

public:

	std::vector<std::vector<int32_t> > m_events;

	struct MTaskProgresses {
		int32_t job_id;
		std::vector<int32_t> blocks;
		std::vector<int32_t> tasks;
		std::vector<af::TaskProgress> tp;
	};
	std::vector<MTaskProgresses> m_tp;

	struct MBlocksIds {
		int32_t job_id;
		int32_t block_num;
		int32_t mode;
	};
	std::vector<MBlocksIds> m_bids;

	std::vector<int32_t> m_jobs_order_ids;

	std::string m_instruction;

	std::vector<MCTask> m_listens;

	std::vector<MCTask> m_outputs;

	std::string m_message;

public:

	bool isEmpty() const;

	void addOutput( const MCTask & i_mctask);

	void addListened( const MCTask & i_mctask);

	void jsonWrite( std::ostringstream & o_str) const;

	void clear();

private:
	void v_readwrite( Msg * msg);
};
}
