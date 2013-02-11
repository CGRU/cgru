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

Service::Service( const std::string & Type,
            const std::string & WDir,
            const std::string & Command,
            const std::string & Files,
            int CapKoeff,
            const std::list<std::string> & Hosts
):
   name( Type),
   wdir( WDir),
   command( Command),
   capkoeff( CapKoeff),
   hosts( Hosts),
   files( Files),
   job_id(0),
   block_id(0),
   task_id(0)
{
   initialize();
}

Service::Service( const TaskExec & taskexec):
   name( taskexec.getServiceType()),
   wdir( taskexec.getWDir()),
   command( taskexec.getCommand()),
   capkoeff( taskexec.getCapCoeff()),
   hosts( taskexec.getMultiHostsNames()),
   files( taskexec.getFiles()),
   job_id(taskexec.getJobId()),
   block_id(taskexec.getBlockNum()),
   task_id(taskexec.getTaskNum())
{
   initialize();
}

void Service::initialize()
{
   PyObj_FuncGetWDir = NULL;
   PyObj_FuncGetCommand = NULL;
   PyObj_FuncGetFiles = NULL;
	PyObj_FuncDoPost = NULL;
   initialized = false;

   PyObject * pHostsList = PyList_New(0);
   for( std::list<std::string>::const_iterator it = hosts.begin(); it != hosts.end(); it++)
      if( PyList_Append( pHostsList, PyBytes_FromString((*it).c_str())) != 0)
      {
         AFERROR("Service::Service: PyList_Append:")
         PyErr_Print();
      }

   PyObject *pArgs;
   pArgs = PyTuple_New( 1);

   PyObject *taskInfo;
   taskInfo = PyDict_New();

   PyDict_SetItemString(taskInfo, "wdir",             PyBytes_FromString( wdir.c_str()));
   PyDict_SetItemString(taskInfo, "command",          PyBytes_FromString( command.c_str()));
   PyDict_SetItemString(taskInfo, "capacity",         PyLong_FromLong( capkoeff));
   PyDict_SetItemString(taskInfo, "hosts",            pHostsList);
   PyDict_SetItemString(taskInfo, "files",            PyBytes_FromString( files.c_str()));

//   PyDict_SetItemString(taskInfo, "thumbnail_cmd",    PyBytes_FromString(Environment::getThumbnailCmd().c_str()));
//   PyDict_SetItemString(taskInfo, "thumbnail_naming", PyBytes_FromString(Environment::getThumbnailNaming().c_str()));
//   PyDict_SetItemString(taskInfo, "thumbnail_http",   PyBytes_FromString(Environment::getThumbnailHttp().c_str()));
//   PyDict_SetItemString(taskInfo, "thumbnail_file",   PyBytes_FromString(Environment::getThumbnailFile().c_str()));
   PyDict_SetItemString(taskInfo, "job_id",           PyLong_FromLong(job_id));
   PyDict_SetItemString(taskInfo, "block_id",         PyLong_FromLong(block_id));
   PyDict_SetItemString(taskInfo, "task_id",          PyLong_FromLong(task_id));

   PyTuple_SetItem( pArgs, 0, taskInfo);

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
			AFERROR("Service:FuncGetCommand: Returned object is not a string.")

		Py_DECREF( pResult);
	}
	else if( PyErr_Occurred())
		PyErr_Print();

	return o_errMsg;
}

