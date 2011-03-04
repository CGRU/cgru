#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
   files( Files)
{
   initialize();
}

Service::Service( const TaskExec & taskexec):
   name( taskexec.getServiceType()),
   wdir( taskexec.getWDir()),
   command( taskexec.getCommand()),
   capkoeff( taskexec.getCapCoeff()),
   hosts( taskexec.getMultiHostsNames()),
   files( taskexec.getFiles())
{
   initialize();
}

void Service::initialize()
{
   PyObj_FuncGetWDir = NULL;
   PyObj_FuncGetCommand = NULL;
   PyObj_FuncGetFiles = NULL;
   PyObj_FuncCheckFiles = NULL;
   initialized = false;

   PyObject * pHostsList = PyList_New(0);
   for( std::list<std::string>::const_iterator it = hosts.begin(); it != hosts.end(); it++)
      if( PyList_Append( pHostsList, PyString_FromString((*it).c_str())) != 0)
      {
         AFERROR("Service::Service: PyList_Append:\n");
         PyErr_Print();
      }

   PyObject *pArgs;
   pArgs = PyTuple_New( 6);
   PyTuple_SetItem( pArgs, 0, PyString_FromString( af::Environment::getAfRoot().c_str()));
   PyTuple_SetItem( pArgs, 1, PyString_FromString( wdir.c_str()));
   PyTuple_SetItem( pArgs, 2, PyString_FromString( command.c_str()));
   PyTuple_SetItem( pArgs, 3, PyInt_FromLong( capkoeff));
   PyTuple_SetItem( pArgs, 4, pHostsList );
   PyTuple_SetItem( pArgs, 5, PyString_FromString( files.c_str()));

   if( PyClass::init( AFPYNAMES::SERVICE_CLASSESDIR, name, pArgs) == false) return;

   //Get functions:
   PyObj_FuncGetWDir= getFunction( AFPYNAMES::SERVICE_FUNC_GETWDIR);
   if( PyObj_FuncGetWDir == NULL ) return;
   PyObj_FuncGetCommand = getFunction( AFPYNAMES::SERVICE_FUNC_GETCOMMAND);
   if( PyObj_FuncGetCommand == NULL ) return;
   PyObj_FuncGetFiles = getFunction( AFPYNAMES::SERVICE_FUNC_GETFILES);
   if( PyObj_FuncGetFiles == NULL ) return;
   PyObj_FuncCheckFiles = getFunction( AFPYNAMES::SERVICE_FUNC_CHECKFILES);
   if( PyObj_FuncCheckFiles == NULL ) return;

   PyObject * pResult;

   // Process working directory:
   pResult = PyObject_CallObject( PyObj_FuncGetWDir, NULL);
   if( PyString_Check( pResult))
   {
      wdir = PyString_AsString( pResult);
      Py_DECREF( pResult);
   }
   else
   {
      AFERROR("Service:FuncGetWDir: Returned object is not a string.\n");
      Py_DECREF( pResult);
      return;
   }

   // Process command:
   pResult = PyObject_CallObject( PyObj_FuncGetCommand, NULL);
   if( PyString_Check( pResult))
   {
      command = PyString_AsString( pResult);
      Py_DECREF( pResult);
   }
   else
   {
      AFERROR("Service:FuncGetCommand: Returned object is not a string.\n");
      Py_DECREF( pResult);
      return;
   }

   // Process files:
   pResult = PyObject_CallObject( PyObj_FuncGetFiles, NULL);
   if( PyString_Check( pResult))
   {
      files = PyString_AsString( pResult);
      Py_DECREF( pResult);
   }
   else
   {
      AFERROR("Service:FuncGetCommand: Returned object is not a string.\n");
      Py_DECREF( pResult);
      return;
   }

   initialized = true;
}

Service::~Service()
{
}

bool Service::checkFiles( int sizemin, int sizemax)
{
   return true;
}
