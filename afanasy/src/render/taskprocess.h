#pragma once

#include "../include/afanasy.h"

#include "../libafanasy/common/dlThread.h"

#include "../libafanasy/msgclasses/mctaskpos.h"

#include "../libafanasy/name_af.h"
#include "../libafanasy/service.h"
#include "../libafanasy/taskexec.h"

class ParserHost;
class RenderHost;

class TaskProcess
{
public:
	TaskProcess( af::TaskExec * i_taskExec, RenderHost * i_render);
	~TaskProcess();

	inline bool is( int i_jobId) const
		{ return m_taskexec->getJobId() == i_jobId ;}

	inline bool is( int i_jobId, int i_blockNum, int i_taskNum, int i_Number) const
		{ return (( m_taskexec->getJobId()    == i_jobId    ) &&
		          ( m_taskexec->getBlockNum() == i_blockNum ) &&
		          ( m_taskexec->getTaskNum()  == i_taskNum  ) &&
		          ( m_taskexec->getNumber()   == i_Number   ));}

	inline bool is( const af::MCTaskPos & i_taskpos) const
		{ return is( i_taskpos.getJobId(), i_taskpos.getBlockNum(), i_taskpos.getTaskNum(), i_taskpos.getNumber());}

	const std::string getOutput() const;

	void refresh();
	void stop();
	void close();

	inline bool isRunning() const { return m_pid != 0;}
	inline bool isClosed()  const { return m_closed;}
	inline bool isZombie()  const { return m_zombie;}

	af::TaskExec * getTaskExec() { return m_taskexec;}

	inline void listenOutput( bool i_subscribe) { m_taskexec->listenOutput( i_subscribe);}

	const std::string generateInfoString( bool i_full = false) const;
	void generateInfoStream( std::ostringstream & o_str, bool i_full = false) const;

	friend std::ostream& operator<<( std::ostream& i_str, const TaskProcess * i_obj) { i_str << i_obj->generateInfoString(); return i_str; }

private:
	void launchCommand();
	void sendTaskSate();
	void readProcess( const std::string & i_mode);
	void processFinished( int i_exitCode);
	void killProcess();
	void closeHandles();
	void collectFiles( af::MCTaskUp & i_task_up);

private:
	RenderHost * m_render;

	af::TaskExec * m_taskexec;
	af::Service * m_service;
	ParserHost * m_parser;

	std::string m_store_dir;
	std::vector<std::string> m_collected_files;
	uint8_t m_update_status;
	time_t m_stop_time;
	bool m_closed;
	bool m_zombie;

	static long long ms_counter;
	int m_dead_cycle;
	int m_cycle;

	std::string m_cmd;
	std::string m_wdir;
	pid_t m_pid;

	int m_commands_launched;

	bool m_doing_post;
	std::vector<std::string> m_post_cmds;

	std::string m_listened;

#ifdef WINNT
	char * m_environ;
	PROCESS_INFORMATION m_pinfo;
	HANDLE m_hjob;
	HANDLE m_io_output;
	HANDLE m_io_outerr;
	HANDLE m_io_input;
	int readPipe( HANDLE i_handle );
#else
	char ** m_environ;
	FILE * m_io_output;
	FILE * m_io_outerr;
	FILE * m_io_input;
	int readPipe( FILE * i_file );
#endif

	// Read buffer:
	static const int m_readbuffer_size = AFRENDER::TASK_READ_BUFFER_SIZE;
	char m_readbuffer[m_readbuffer_size];
	// Will be set to FILEs:
	char m_filebuffer_out[m_readbuffer_size];
	char m_filebuffer_err[m_readbuffer_size];
};
