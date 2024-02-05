#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/afanasy.h"
#include "../include/afpynames.h"

#include "environment.h"
#include "logger.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Service::Service(
	const TaskExec * i_task_exec,
	const std::string & i_store_dir
):
	m_name( i_task_exec->getServiceType()),
	m_parser_type( i_task_exec->getParserType()),
	m_wdir( i_task_exec->getWDir())
{
	initialize( i_task_exec, i_store_dir);
}

Service::Service(
	const std::string & i_type,
	const std::string & i_wdir,
	const std::string & i_command_task
):
	m_name( i_type),
	m_parser_type("generic"),
	m_wdir( i_wdir)
{
	TaskExec * i_task_exec = new TaskExec(
			"name",m_name, m_parser_type,
			1,
            i_command_task,
			std::vector<std::string>(),
			1, 1, 1, 1,
			m_wdir,
			std::map<std::string,std::string>(),
			1, 0, 0, 1
		);
	initialize( i_task_exec, "");
	delete i_task_exec;
}

Service::Service(
	const std::string & i_type,
	const std::string & i_parser_type
):
	m_name( i_type),
	m_parser_type( i_parser_type)
{
	TaskExec * i_task_exec = new TaskExec(
			"name", m_name, m_parser_type,
			1,
			"",
            std::vector<std::string>(),
			1, 1, 1, 1,
			m_wdir,
			std::map<std::string,std::string>(),
			1, 0, 0, 1
		);
	initialize( i_task_exec, "");
	delete i_task_exec;
}

Service::Service(
	const std::string & i_wdir
):
	m_name("generic"),
	m_parser_type("generic"),
	m_wdir( i_wdir)
{
	TaskExec * i_task_exec = new TaskExec(
			"name", m_name, m_parser_type,
			1,
			"",
            std::vector<std::string>(),
			1, 1, 1, 1,
			m_wdir,
			std::map<std::string,std::string>(),
			1, 0, 0, 1
		);
	initialize( i_task_exec, "");
	delete i_task_exec;
}

Service::Service(
	const std::vector<std::string> & i_files_block,
	long long i_frame_start, long long i_frame_end, long long i_frame_inc,
	const std::vector<std::string> & i_files_task
):
	m_name("generic"),
	m_parser_type("generic")
{
	long long block_flags = 0;
	if(i_files_task.size() == 0)
		block_flags |= BlockData::FNumeric;

	TaskExec * task_exec = new TaskExec(
			"name", m_name, m_parser_type,
			1,
            "",
			i_files_block,
			i_frame_start, i_frame_end, i_frame_inc, 1,
			m_wdir,
			std::map<std::string,std::string>(),
			1, 0, block_flags, 1
		);

	task_exec->setTaskFiles(i_files_task);
	initialize(task_exec,"");

	delete task_exec;
}

Service::Service(
	const std::string & i_type,
	const std::string & i_command_block,
	long long i_frame_start, long long i_frame_end
):
	m_name(i_type),
	m_parser_type("generic")
{
	TaskExec * i_task_exec = new TaskExec(
			"name", m_name, m_parser_type,
			1,
			i_command_block,
            std::vector<std::string>(),
			i_frame_start, i_frame_end, 1, 1,
			m_wdir,
			std::map<std::string,std::string>(),
			1, 0, BlockData::FNumeric, 1
		);
	initialize( i_task_exec, "");
	delete i_task_exec;
}

void Service::initialize( const TaskExec * i_task_exec, const std::string & i_store_dir)
{
	m_PyObj_FuncSkipTask = NULL;
	m_PyObj_FuncGetWDir = NULL;
	m_PyObj_FuncGetCommand = NULL;
	m_PyObj_FuncGetEnvironment = NULL;
	m_PyObj_FuncGetFiles = NULL;
	m_PyObj_FuncGetParsedFiles = NULL;
	m_PyObj_FuncParse = NULL;
	m_PyObj_FuncCheckRenderedFiles = NULL;
	m_PyObj_FuncCheckExitStatus = NULL;
	m_PyObj_FuncDoPost = NULL;
	m_initialized = false;

	PyObject * pFilesBlockList = PyList_New(0);
	for (int i = 0; i < i_task_exec->getFilesBlock().size(); i++)
		PyList_Append(pFilesBlockList, PyBytes_FromString(i_task_exec->getFilesBlock()[i].c_str() ));
	PyObject * pFilesTaskList = PyList_New(0);
	for (int i = 0; i < i_task_exec->getFilesTask().size(); i++)
		PyList_Append(pFilesTaskList, PyBytes_FromString(i_task_exec->getFilesTask()[i].c_str() ));

	PyObject * pParsedFilesList = PyList_New(0);
	for( int i = 0; i < i_task_exec->getParsedFiles().size(); i++)
		PyList_Append( pParsedFilesList, PyBytes_FromString( i_task_exec->getParsedFiles()[i].c_str() ));

	PyObject * pHostsList = PyList_New(0);
	for( std::list<std::string>::const_iterator it = i_task_exec->getMultiHostsNames().begin();
			it != i_task_exec->getMultiHostsNames().end(); it++)
		PyList_Append( pHostsList, PyBytes_FromString((*it).c_str()));


	PyObject * pEenvDict = PyDict_New();
	for (auto const& it : i_task_exec->getEnv())
		PyDict_SetItemString(pEenvDict, it.first.c_str(), PyBytes_FromString(it.second.c_str()));

	PyObject * pTicketsDict = PyDict_New();
	for (auto const& it : i_task_exec->getTickets())
		PyDict_SetItemString(pTicketsDict, it.first.c_str(), PyLong_FromLong(it.second));

	PyObject *pArgs;
	pArgs = PyTuple_New( 2);

	PyObject *task_info;
	task_info = PyDict_New();

	PyDict_SetItemString( task_info, "wdir",          PyBytes_FromString( i_task_exec->getWDir().c_str()));
	PyDict_SetItemString( task_info, "parser",        PyBytes_FromString( m_parser_type.c_str()));
	PyDict_SetItemString(task_info, "command_block",  PyBytes_FromString(i_task_exec->getCommandBlock().c_str()));
	PyDict_SetItemString(task_info, "command_task",   PyBytes_FromString(i_task_exec->getCommandTask().c_str()));
	PyDict_SetItemString(task_info, "capacity",       PyLong_FromLong(i_task_exec->getCapacity()));
	PyDict_SetItemString(task_info, "capacity_coeff", PyLong_FromLong(i_task_exec->getCapCoeff()));
	PyDict_SetItemString(task_info, "files_block",    pFilesBlockList);
	PyDict_SetItemString(task_info, "files_task",     pFilesTaskList);
	PyDict_SetItemString( task_info, "hosts",         pHostsList);
	PyDict_SetItemString( task_info, "parsed_files",  pParsedFilesList);
	PyDict_SetItemString(task_info, "environment",    pEenvDict);
	PyDict_SetItemString(task_info, "tickets",        pTicketsDict);

    PyDict_SetItemString( task_info, "frame_start",  PyLong_FromLong(i_task_exec->getFrameStart()));
    PyDict_SetItemString( task_info, "frame_finish", PyLong_FromLong(i_task_exec->getFrameFinish()));
    PyDict_SetItemString( task_info, "frame_inc",    PyLong_FromLong(i_task_exec->getFrameInc()));
    PyDict_SetItemString( task_info, "frames_num",   PyLong_FromLong(i_task_exec->getFramesNum()));

	PyDict_SetItemString( task_info, "task_id",          PyLong_FromLong( i_task_exec->getTaskNum()));
	PyDict_SetItemString( task_info, "task_name",        PyBytes_FromString( i_task_exec->getName().c_str()));

	PyDict_SetItemString( task_info, "block_id",          PyLong_FromLong( i_task_exec->getBlockNum()));
	PyDict_SetItemString( task_info, "block_name",        PyBytes_FromString( i_task_exec->getBlockName().c_str()));
	PyDict_SetItemString( task_info, "block_flags",       PyLong_FromLong( i_task_exec->getBlockFlags()));

	PyDict_SetItemString( task_info, "job_id",          PyLong_FromLong( i_task_exec->getJobId()));
	PyDict_SetItemString( task_info, "job_name",        PyBytes_FromString( i_task_exec->getJobName().c_str()));

	PyDict_SetItemString( task_info, "user_name",        PyBytes_FromString( i_task_exec->getUserName().c_str()));

	PyDict_SetItemString( task_info, "store_dir", PyBytes_FromString( i_store_dir.c_str()));

	for (auto const& it : i_task_exec->getDataIntegers())
		PyDict_SetItemString(task_info, it.first.c_str(), PyLong_FromLong(it.second));
	for (auto const& it : i_task_exec->getDataStrings())
		PyDict_SetItemString(task_info, it.first.c_str(), PyBytes_FromString(it.second.c_str()));

	PyTuple_SetItem( pArgs, 0, task_info);
	PyTuple_SetItem( pArgs, 1, PyBool_FromLong( af::Environment::isVerboseMode()));

	// Try to import service class
	if( false == PyClass::init( AFPYNAMES::SERVICE_CLASSESDIR, m_name, pArgs))
		// If failed and imported class was not the base class
		if( m_name != AFPYNAMES::SERVICE_CLASSESBASE )
			// Try to import base service
			if( false == PyClass::init( AFPYNAMES::SERVICE_CLASSESDIR, AFPYNAMES::SERVICE_CLASSESBASE, pArgs))
				return;

	//Get functions:
	m_PyObj_FuncSkipTask = getFunction(AFPYNAMES::SERVICE_FUNC_SKIPTASK);
	if (m_PyObj_FuncSkipTask == NULL)
		return;

	m_PyObj_FuncGetWDir= getFunction( AFPYNAMES::SERVICE_FUNC_GETWDIR);
	if( m_PyObj_FuncGetWDir == NULL ) return;

	m_PyObj_FuncGetCommand = getFunction( AFPYNAMES::SERVICE_FUNC_GETCOMMAND);
	if( m_PyObj_FuncGetCommand == NULL ) return;

	m_PyObj_FuncGetEnvironment = getFunction(AFPYNAMES::SERVICE_FUNC_GETENVIRONMENT);
	if (NULL == m_PyObj_FuncGetEnvironment)
		return;

	m_PyObj_FuncGetFiles = getFunction( AFPYNAMES::SERVICE_FUNC_GETFILES);
	if( m_PyObj_FuncGetFiles == NULL ) return;

	m_PyObj_FuncGetParsedFiles = getFunction( AFPYNAMES::SERVICE_FUNC_GETPARSEDFILES);
	if( m_PyObj_FuncGetParsedFiles == NULL ) return;

	m_PyObj_FuncHasParser = getFunction( AFPYNAMES::SERVICE_FUNC_HASPARSER);
	if( m_PyObj_FuncHasParser == NULL ) return;

	m_PyObj_FuncParse = getFunction( AFPYNAMES::SERVICE_FUNC_PARSE);
	if( m_PyObj_FuncParse == NULL ) return;

	m_PyObj_FuncToHTML = getFunction( AFPYNAMES::SERVICE_FUNC_TOHTML);
	if( m_PyObj_FuncToHTML == NULL ) return;

	m_PyObj_FuncGetLog = getFunction( AFPYNAMES::SERVICE_FUNC_GETLOG);
	if( m_PyObj_FuncGetLog == NULL ) return;

	m_PyObj_FuncCheckExitStatus = getFunction( AFPYNAMES::SERVICE_FUNC_CHECKEXITSTATUS);
	if( m_PyObj_FuncParse == NULL ) return;

	m_PyObj_FuncCheckRenderedFiles = getFunction( AFPYNAMES::SERVICE_FUNC_CHECKRENDEREDFILES);
	if( m_PyObj_FuncCheckRenderedFiles == NULL ) return;

	m_PyObj_FuncDoPost = getFunction( AFPYNAMES::SERVICE_FUNC_DOPOST);
	if( m_PyObj_FuncDoPost == NULL ) return;

	m_PyObj_FuncDoPostLimitSec = getFunction(AFPYNAMES::SERVICE_FUNC_DOPOSTLIMITSEC);
	if (m_PyObj_FuncDoPostLimitSec == NULL)
		return;

	PyObject * pResult;

	// Process working directory:
	AFINFA("Service::initialize: Processing working dirctory:\n%s", m_wdir.c_str())
	pResult = PyObject_CallObject( m_PyObj_FuncGetWDir, NULL);
	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return;
	}
	if( false == af::PyGetString( pResult, m_wdir))
	{
		AFERROR("Service:FuncGetWDir: Returned object is not a string.")
		Py_DECREF( pResult);
		return;
	}
	Py_DECREF( pResult);

	// Process command:
	pResult = PyObject_CallObject( m_PyObj_FuncGetCommand, NULL);
	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return;
	}
	if( false == af::PyGetString( pResult, m_command))
	{
		AFERROR("Service:FuncGetCommand: Returned object is not a string.")
		Py_DECREF( pResult);
		return;
	}
	Py_DECREF( pResult);

	// Process environment:
	pResult = PyObject_CallObject(m_PyObj_FuncGetEnvironment, NULL);
	if (NULL == pResult)
	{
		if (PyErr_Occurred())
			PyErr_Print();
		return;
	}
	if (false == af::PyGetDict(pResult, m_environment))
	{
		AF_ERR << "Service: Returned object is not a dict.";
		Py_DECREF(pResult);
		return;
	}
	Py_DECREF(pResult);

	m_initialized = true;
}

Service::~Service()
{
}

bool Service::skipTask() const
{
	PyObject * pResult = PyObject_CallObject(m_PyObj_FuncSkipTask, NULL);
	if (pResult == NULL)
	{
		if (PyErr_Occurred())
			PyErr_Print();
		return false;
	}

	if (true != PyBool_Check(pResult))
	{
		AF_ERR << "Return object type is not a boolean.";
		return false;
	}

	bool result = PyObject_IsTrue(pResult);

	Py_DECREF(pResult);

	return result;
}

bool Service::hasParser() const
{
	PyObject * pResult = PyObject_CallObject( m_PyObj_FuncHasParser, NULL);

	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return false;
	}

	if( true != PyBool_Check( pResult))
	{
		AFERROR("Service::checkExitStatus: Return object type is not a boolean.")
		Py_DECREF( pResult);
		return false;
	}

	bool result = PyObject_IsTrue( pResult);

	Py_DECREF( pResult);

	return result;
}

void Service::parse(const std::string & i_mode, int i_pid,
		std::string & io_data, std::string & io_resources,
		int & o_percent, int & o_frame, int & o_percentframe,
		std::string & o_activity, std::string & o_report,
		bool & o_progress_changed,
		bool & o_warning, bool & o_error, bool & o_badresult, bool & o_finishedsuccess) const
{
	PyObject * pArgs = PyTuple_New(1);
	PyObject * dict = PyDict_New();
	PyTuple_SetItem(pArgs, 0, dict);

	PyDict_SetItemString(dict, "mode",      PyBytes_FromStringAndSize(i_mode.data(), i_mode.size()));
	PyDict_SetItemString(dict, "pid",       PyLong_FromLong(i_pid));
	PyDict_SetItemString(dict, "data",      PyBytes_FromStringAndSize(io_data.data(), io_data.size()));
	PyDict_SetItemString(dict, "resources", PyBytes_FromStringAndSize(io_resources.data(), io_resources.size()));

	io_resources.clear();

	PyObject * pClass = PyObject_CallObject(m_PyObj_FuncParse, pArgs);
	if (pClass != NULL)
	{
		if (pClass != Py_None )
		{
			std::string err = std::string("Service::parse[" + m_parser_type + "]: ");

			af::PyGetAttrInt(pClass,"percent",      o_percent,      err);
			af::PyGetAttrInt(pClass,"frame",        o_frame,        err);
			af::PyGetAttrInt(pClass,"percentframe", o_percentframe, err);

			af::PyGetAttrBool(pClass,"progress_changed",o_progress_changed,err);
			af::PyGetAttrBool(pClass,"warning",         o_warning,         err);
			af::PyGetAttrBool(pClass,"error",           o_error,           err);
			af::PyGetAttrBool(pClass,"badresult",       o_badresult,       err);
			af::PyGetAttrBool(pClass,"finishedsuccess", o_finishedsuccess, err);

			af::PyGetAttrStr(pClass,"activity",  o_activity,  err);
			af::PyGetAttrStr(pClass,"resources",io_resources, err);
			af::PyGetAttrStr(pClass,"report",    o_report,    err);

			PyObject * pAttr = PyObject_GetAttrString(pClass, "result");
			if (pAttr && (pAttr != Py_None))
				af::PyGetString(pAttr, io_data, "Service::parse: result");
		}
	}
	else
	{
		if (PyErr_Occurred())
			PyErr_Print();
	}

	Py_DECREF(pArgs);
}

const std::string Service::toHTML( const std::string & i_data) const
{
	PyObject * pArgs = PyTuple_New( 1);
	PyTuple_SetItem( pArgs, 0, PyBytes_FromStringAndSize( i_data.c_str(), i_data.size()));

	PyObject * pResult = PyObject_CallObject( m_PyObj_FuncToHTML, pArgs);

	if( pResult == NULL)
	{
		if( PyErr_Occurred())
			PyErr_Print();
		else
			AF_ERR << "Result is NULL.";
		return i_data;
	}


	std::string text;
	if( false == af::PyGetString( pResult, text,"Service::toHTML"))
	{
		AF_ERR << "Can't get string from return object.";
		return i_data;
	}

	Py_DECREF( pResult);

	return text;
}

const std::string Service::getLog() const
{
	std::string log;

	PyObject * pResult = PyObject_CallObject( m_PyObj_FuncGetLog, NULL);
	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return log;
	}

	af::PyGetString( pResult, log);

	Py_DECREF( pResult);
	return log;
}

bool Service::checkExitStatus( int i_status) const
{
	PyObject * pArgs = PyTuple_New( 1);
	PyTuple_SetItem( pArgs, 0, PyLong_FromLong( i_status));

	PyObject * pResult = PyObject_CallObject( m_PyObj_FuncCheckExitStatus, pArgs);

	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return true;
	}

	if( true != PyBool_Check( pResult))
	{
		AFERROR("Service::checkExitStatus: Return object type is not a boolean.")
		return true;
	}

	bool result = PyObject_IsTrue( pResult);

	Py_DECREF( pResult);

	//printf("Service::checkExitStatus: %d %d\n", i_status, result);
	return result;
}

bool Service::checkRenderedFiles() const
{
	PyObject * pResult = PyObject_CallObject( m_PyObj_FuncCheckRenderedFiles, NULL);
	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return true;
	}

	if( true != PyBool_Check( pResult))
	{
		AFERROR("Service::checkRenderedFiles: Return object type is not a boolean.")
		return true;
	}

	bool result = PyObject_IsTrue( pResult);

	Py_DECREF( pResult);

	return result;
}

const std::vector<std::string> Service::doPost()
{
	AFINFA("Service::doPost()")

	std::vector<std::string> cmds;

	PyObject * pResult = PyObject_CallObject(m_PyObj_FuncDoPost, NULL);
	if( pResult )
	{
		if( false == af::PyGetStringList( pResult, cmds))
			AFERRAR("Service:doPost: '%s': returned object is not a string.", m_name.c_str())

		Py_DECREF( pResult);
	}
	else if( PyErr_Occurred())
		PyErr_Print();

	return cmds;
}

int Service::doPostLimitSec()
{
	AFINFA("Service::doPostLimitSec()")

	int limit_sec = 0;

	PyObject * pResult = PyObject_CallObject(m_PyObj_FuncDoPostLimitSec, NULL);
	if (pResult)
	{
		if( ! PyLong_Check(pResult))
			AFERRAR("Service:doPostLimitSec: '%s': returned object is not an integer.", m_name.c_str())

		limit_sec = PyLong_AsLong(pResult);

		Py_DECREF(pResult);
	}
	else if (PyErr_Occurred())
		PyErr_Print();

	return limit_sec;
}

const std::vector<std::string> Service::getFiles() const
{
	std::vector<std::string> files;

	PyObject * pResult = PyObject_CallObject( m_PyObj_FuncGetFiles, NULL);
	if( pResult )
	{
		if( false == af::PyGetStringList( pResult, files))
			AFERRAR("Service:getFiles: '%s': returned object is not a string.", m_name.c_str())

		Py_DECREF( pResult);
	}
	else if( PyErr_Occurred())
		PyErr_Print();

	return files;
}

const std::vector<std::string> Service::getParsedFiles() const
{
//printf("Service::getParsedFiles():\n");
	std::vector<std::string> files;

	PyObject * pResult = PyObject_CallObject( m_PyObj_FuncGetParsedFiles, NULL);
	if( pResult )
	{
		if( false == af::PyGetStringList( pResult, files))
			AFERRAR("Service:getParsedFiles: '%s': returned object is not a string.", m_name.c_str())

		Py_DECREF( pResult);
	}
	else if( PyErr_Occurred())
		PyErr_Print();

	return files;
}

