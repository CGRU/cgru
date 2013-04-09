#pragma once

#include "name_af.h"
#include "pyclass.h"
#include "taskexec.h"

namespace af
{
class Service: public PyClass
{
public:
	Service( const std::string & i_type,
	         const std::string & i_wdir,
	         const std::string & i_command,
	         const std::string & i_files = std::string()
		);
	Service( const TaskExec & taskexec);
	~Service();

	inline bool isInitialized() const { return initialized;}

	const std::string getWDir()    const { return wdir;    }
	const std::string getCommand() const { return command; }
	const std::string getFiles() const   { return files;   }

	// Return an empty string on sucess or an error message on error
	const std::string doPost();

private:
	void initialize( const TaskExec & taskExec);

private:
	std::string name;

	PyObject* PyObj_FuncGetWDir;
	PyObject* PyObj_FuncGetCommand;
	PyObject* PyObj_FuncGetFiles;
	PyObject* PyObj_FuncDoPost;

	bool initialized;

	std::string wdir;
	std::string command;
	std::string files;
};
}
