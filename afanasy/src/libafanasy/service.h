#pragma once

#include "name_af.h"
#include "pyclass.h"
#include "taskexec.h"

namespace af
{
class Service: public PyClass
{
public:
	Service( const TaskExec * taskexec, const std::string & i_store_dir = std::string());

	Service( /* For afrender to generate WOL sleep command */
			const std::string & i_type,
			const std::string & i_wdir,
			const std::string & i_command_task
		);

	Service( /* For afwatch to parse task output: */
			const std::string & i_type,
			const std::string & i_parser_type
		);

	Service( /* For afwatch to paths map folder: */
			const std::string & i_wdir
		);

	Service( /* For afwatch to generate and paths map files: */
			const std::vector<std::string> & i_files_block,
			long long i_frame_start, long long i_frame_end, long long i_frame_inc,
			const std::vector<std::string> & i_files_task = std::vector<std::string>()
		);

	Service( /* For afcmd to test service on a numeric command: */
			const std::string & i_type,
			const std::string & i_command_block,
			long long i_frame_start, long long i_frame_end
		);

	~Service();

	inline bool isInitialized() const { return m_initialized;}

	bool skipTask() const;
	inline const std::string & getWDir()    const { return m_wdir;    }
	inline const std::string & getCommand() const { return m_command; }
	const std::map<std::string, std::string> & getEnvironment() const {return m_environment;}
	const std::vector<std::string> getFiles() const;
	const std::vector<std::string> getParsedFiles() const;

	bool hasParser() const;

	void parse (const std::string & i_mode, int i_pid,
				std::string & io_data, std::string & io_resources,
				int & o_percent, int & o_frame, int & o_percentframe,
				std::string & o_activity, std::string & o_report,
				bool & o_progress_changed,
				bool & o_warning, bool & o_error, bool & o_badresult, bool & o_finishedsuccess) const;

	const std::string toHTML( const std::string & i_data) const;

	const std::string getLog() const;

	bool checkRenderedFiles() const;
	bool checkExitStatus( int i_status) const;

	// Return an empty string on sucess or an error message on error
	const std::vector<std::string> doPost();
	int doPostLimitSec();

private:
	void initialize( const TaskExec * taskExec, const std::string & i_store_dir);

private:
	std::string m_name;
	std::string m_parser_type;

	PyObject * m_PyObj_FuncSkipTask;
	PyObject * m_PyObj_FuncGetWDir;
	PyObject * m_PyObj_FuncGetCommand;
	PyObject * m_PyObj_FuncGetEnvironment;
	PyObject * m_PyObj_FuncGetFiles;
	PyObject * m_PyObj_FuncGetParsedFiles;
	PyObject * m_PyObj_FuncHasParser;
	PyObject * m_PyObj_FuncParse;
	PyObject * m_PyObj_FuncToHTML;
	PyObject * m_PyObj_FuncGetLog;
	PyObject * m_PyObj_FuncCheckExitStatus;
	PyObject * m_PyObj_FuncCheckRenderedFiles;
	PyObject * m_PyObj_FuncDoPost;
	PyObject * m_PyObj_FuncDoPostLimitSec;

	bool m_initialized;

	std::string m_wdir;
	std::string m_command;
	std::map<std::string, std::string> m_environment;
};
}
