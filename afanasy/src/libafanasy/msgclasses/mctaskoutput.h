#pragma once

#include "../msg.h"
#include "../name_af.h"

#include "mctaskpos.h"
#include "msgclass.h"

namespace af
{

class MCTaskOutput : public MsgClass
{
public:
	MCTaskOutput( int i_job_id = 0, int i_block_id = 0, int i_task_id = 0, int i_start_num = 0);
	MCTaskOutput( Msg * msg);
	~MCTaskOutput();

	Msg * generateMessage( bool i_binary);

	bool isSameTask( const MCTaskOutput & i_other) const;
	bool isSameTask( const MCTaskPos    & i_tp   ) const;

	virtual void v_readwrite( Msg * io_msg);

	void jsonWrite( std::ostringstream & o_str) const;

	void v_generateInfoStream( std::ostringstream & o_str, bool i_full = false) const;

public:
	int32_t m_job_id;
	int32_t m_block_id;
	int32_t m_task_id;
	int32_t m_start_num;
	int32_t m_render_id;

	std::string m_job_name;
	std::string m_block_name;
	std::string m_task_name;
	std::string m_service;
	std::string m_parser;

	std::string m_filename;

	std::string m_output;

private:

};
}
