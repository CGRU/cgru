#pragma once

#include "../msg.h"
#include "../name_af.h"
#include "../taskprogress.h"

#include "mctaskpos.h"
#include "msgclass.h"

namespace af
{

/// This class designed to send any task data to GUI

class MCTask : public MsgClass
{
public:
	MCTask( int i_job_id = 0, int i_block_id = 0, int i_task_id = 0, int i_start_num = 0);
	MCTask( Msg * msg);
	~MCTask();

	enum Type
	{
		TNULL,
		TExec,
		TOutput,
		TLog,
		TErrorHosts,
		TListen,
		TLAST
	};

	static const char * TNAMES[];

	inline bool isEmpty() const { return m_type == TNULL; }
	inline const int32_t getType() const { return m_type; }


	inline bool hasOutput() const { return ( m_type == TOutput ) && ( m_data.size() > 0 ); }
	inline bool hasExec()   const { return ( m_type == TExec   ) && ( m_exec != NULL    ); }


	// Task position:
	//
	inline bool isSameTask( const MCTask & i_other ) const { return m_pos.isSameTask( i_other.m_pos);}
	inline bool isSameTask( const MCTaskPos & i_pos) const { return m_pos.isSameTask(         i_pos);}
	inline const int32_t getJobId()    const { return m_pos.getJobId();   }
	inline const int32_t getBlockNum() const { return m_pos.getBlockNum();}
	inline const int32_t getTaskNum()  const { return m_pos.getTaskNum(); }
	inline const int32_t getNumber()   const { return m_pos.getNumber();  }
	inline const MCTaskPos & getPos()  const { return m_pos; }


	// Task executable mode:
	//
	// This function takes af::TaskExec ownership.
	// It will be deleted in dtor.
	void setExec( TaskExec * i_exec); 

	// This function release af::TaskExec ownership.
	// You can use it after class dtor.
	// You should delete it manually when not needed.
	TaskExec * getExec();


	// Task output mode:
	//
	void setOutput( const std::string & i_output);
	void updateOutput( const std::string & i_output);
	const std::string & getOutput() const;


	// Task log mode:
	//
	void setLog( const std::list<std::string> & i_list);
	const std::string & getLog() const;


	// Task error hosts mode:
	//
	void setErrorHosts( const std::list<std::string> & i_list);
	const std::string & getErrorHosts() const;


	// Task listen mode:
	//
	void setListened( const std::string & i_output);
	void appendListened( const std::string & i_output);
	const std::string & getListened() const;


	// Read/write:
	//
	Msg * generateMessage( bool i_binary);
	void jsonWrite( std::ostringstream & o_str) const;
	virtual void v_readwrite( Msg * io_msg);

public:
	int32_t m_render_id;

	std::string m_job_name;
	std::string m_block_name;
	std::string m_task_name;
	std::string m_service;
	std::string m_parser;

	af::TaskProgress m_progress;

private:
	void init();
	virtual void v_generateInfoStream( std::ostringstream & o_str, bool i_full = false) const;

private:
	int32_t m_type;

	af::MCTaskPos m_pos;

	std::string m_data;

	af::TaskExec * m_exec;
};
}
