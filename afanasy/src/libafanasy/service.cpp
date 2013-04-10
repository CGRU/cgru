#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/afanasy.h"
#include "../include/afpynames.h"

#include "../libafanasy/environment.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Service::Service( const std::string & i_type,
				const std::string & i_wdir,
				const std::string & i_command,
				const std::string & i_files
):
	name( i_type),
	wdir( i_wdir),
	command( i_command),
	files( i_files)
{
	TaskExec * task_exec = new TaskExec(
			"i_name", i_type, "", i_command,
			1, -1, -1,
			i_files,
			1, 1, 1,
			i_wdir,
			"", 1, 0, 0, 1
		);
	initialize( *task_exec);
	delete task_exec;
}

Service::Service( const TaskExec & task_exec):
	name( task_exec.getServiceType()),
	wdir( task_exec.getWDir()),
	command( task_exec.getCommand()),
	files( task_exec.getFiles())
{
	initialize( task_exec);
}

void Service::initialize( const TaskExec & task_exec)
{
	PyObj_FuncGetWDir = NULL;
	PyObj_FuncGetCommand = NULL;
	PyObj_FuncGetFiles = NULL;
	PyObj_FuncDoPost = NULL;
	initialized = false;

	PyObject * pHostsList = PyList_New(0);
	for( std::list<std::string>::const_iterator it = task_exec.getMultiHostsNames().begin(); it != task_exec.getMultiHostsNames().end(); it++)
		if( PyList_Append( pHostsList, PyBytes_FromString((*it).c_str())) != 0)
		{
			AFERROR("Service::Service: PyList_Append:")
			PyErr_Print();
		}

	PyObject *pArgs;
	pArgs = PyTuple_New( 1);

	PyObject *task_info;
	task_info = PyDict_New();

	PyDict_SetItemString( task_info, "wdir",     PyBytes_FromString( task_exec.getWDir().c_str()));
	PyDict_SetItemString( task_info, "command",  PyBytes_FromString( task_exec.getCommand().c_str()));
	PyDict_SetItemString( task_info, "capacity", PyLong_FromLong( task_exec.getCapCoeff()));
	PyDict_SetItemString( task_info, "files",    PyBytes_FromString( task_exec.getFiles().c_str()));
	PyDict_SetItemString( task_info, "hosts",    pHostsList);

	PyDict_SetItemString( task_info, "task_id",          PyLong_FromLong( task_exec.getTaskNum()));
	PyDict_SetItemString( task_info, "task_name",        PyBytes_FromString( task_exec.getName().c_str()));
	PyDict_SetItemString( task_info, "task_custom_data", PyBytes_FromString( task_exec.m_custom_data_task.c_str()));

	PyDict_SetItemString( task_info, "block_id",          PyLong_FromLong( task_exec.getBlockNum()));
	PyDict_SetItemString( task_info, "block_name",        PyBytes_FromString( task_exec.getBlockName().c_str()));
	PyDict_SetItemString( task_info, "block_flags",       PyLong_FromLong( task_exec.getBlockFlags()));
	PyDict_SetItemString( task_info, "block_custom_data", PyBytes_FromString( task_exec.m_custom_data_block.c_str()));

	PyDict_SetItemString( task_info, "job_id",          PyLong_FromLong( task_exec.getJobId()));
	PyDict_SetItemString( task_info, "job_name",        PyBytes_FromString( task_exec.getJobName().c_str()));
	PyDict_SetItemString( task_info, "job_flags",       PyLong_FromLong( task_exec.getJobFlags()));
	PyDict_SetItemString( task_info, "job_custom_data", PyBytes_FromString( task_exec.m_custom_data_job.c_str()));

	PyDict_SetItemString( task_info, "user_name",        PyBytes_FromString( task_exec.getUserName().c_str()));
	PyDict_SetItemString( task_info, "user_flags",       PyLong_FromLong( task_exec.getUserFlags()));
	PyDict_SetItemString( task_info, "user_custom_data", PyBytes_FromString( task_exec.m_custom_data_user.c_str()));

	PyDict_SetItemString( task_info, "render_flags",       PyLong_FromLong( task_exec.getRenderFlags()));
	PyDict_SetItemString( task_info, "render_custom_data", PyBytes_FromString( task_exec.m_custom_data_render.c_str()));

	PyTuple_SetItem( pArgs, 0, task_info);

	// Try to import service class
	if( false == PyClass::init( AFPYNAMES::SERVICE_CLASSESDIR, name, pArgs))
		// If failed and imported class was not the base class
		if( name != AFPYNAMES::SERVICE_CLASSESBASE)
			// Try to import base service
			if( false == PyClass::init( AFPYNAMES::SERVICE_CLASSESDIR, AFPYNAMES::SERVICE_CLASSESBASE, pArgs))
				return;

	//Get functions:
	PyObj_FuncGetWDir= getFunction( AFPYNAMES::SERVICE_FUNC_GETWDIR);
	if( PyObj_FuncGetWDir == NULL ) return;
	PyObj_FuncGetCommand = getFunction( AFPYNAMES::SERVICE_FUNC_GETCOMMAND);
	if( PyObj_FuncGetCommand == NULL ) return;
	PyObj_FuncGetFiles = getFunction( AFPYNAMES::SERVICE_FUNC_GETFILES);
	if( PyObj_FuncGetFiles == NULL ) return;
	PyObj_FuncDoPost = getFunction( AFPYNAMES::SERVICE_FUNC_DOPOST);
	if( PyObj_FuncDoPost == NULL ) return;

	PyObject * pResult;

	// Process working directory:
	AFINFA("Service::initialize: Processing working dirctory:\n%s", wdir.c_str())
	pResult = PyObject_CallObject( PyObj_FuncGetWDir, NULL);
	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return;
	}
	if( false == af::PyGetString( pResult, wdir))
	{
		AFERROR("Service:FuncGetWDir: Returned object is not a string.")
		Py_DECREF( pResult);
		return;
	}
	Py_DECREF( pResult);
	AFINFA("Service::initialize: Working dirctory:\n%s", wdir.c_str())

	// Process command:
	AFINFA("Service::initialize: Processing command:\n%s", command.c_str())
	pResult = PyObject_CallObject( PyObj_FuncGetCommand, NULL);
	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return;
	}
	if( false == af::PyGetString( pResult, command))
	{
		AFERROR("Service:FuncGetCommand: Returned object is not a string.")
		Py_DECREF( pResult);
		return;
	}
	Py_DECREF( pResult);
	AFINFA("Service::initialize: Command:\n%s", command.c_str())

	// Process files:
	AFINFA("Service::initialize: Processing files:\n%s", files.c_str())
	pResult = PyObject_CallObject( PyObj_FuncGetFiles, NULL);
	if( pResult == NULL)
	{
		if( PyErr_Occurred()) PyErr_Print();
		return;
	}
	if( false == af::PyGetString( pResult, files))
	{
		AFERROR("Service:FuncGetCommand: Returned object is not a string.")
		Py_DECREF( pResult);
		return;
	}
	Py_DECREF( pResult);
	AFINFA("Service::initialize: Files:\n%s", files.c_str())

	initialized = true;
}

Service::~Service()
{
}

const std::string Service::doPost()
{
	AFINFA("Service::doPost()")

	std::string o_errMsg;

	PyObject * pResult = PyObject_CallObject( PyObj_FuncDoPost, NULL);
	if( pResult )
	{
		if( false == af::PyGetString( pResult, o_errMsg))
			AFERROR("Service:FuncDoPost: Returned object is not a string.")

		Py_DECREF( pResult);
	}
	else if( PyErr_Occurred())
		PyErr_Print();

	return o_errMsg;
}

