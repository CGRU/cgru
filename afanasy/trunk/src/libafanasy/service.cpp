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

Service::Service( const QString & Type,
            const QString & WDir,
            const QString & Command,
            const QString & Files,
            int CapKoeff,
            const QStringList & Hosts
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
   for( int h = 0; h < hosts.size(); h++)
      if( PyList_Append( pHostsList , PyString_FromString( hosts[h].toUtf8().data())) != 0)
      {
         AFERROR("Service::Service: PyList_Append:\n");
         PyErr_Print();
      }

   PyObject *pArgs;
   pArgs = PyTuple_New( 6);
   PyTuple_SetItem( pArgs, 0, PyString_FromString( af::Environment::getAfRoot().c_str()));
   PyTuple_SetItem( pArgs, 1, PyString_FromString( wdir.toUtf8().data()));
   PyTuple_SetItem( pArgs, 2, PyString_FromString( command.toUtf8().data()));
   PyTuple_SetItem( pArgs, 3, PyInt_FromLong( capkoeff));
   PyTuple_SetItem( pArgs, 4, pHostsList );
   PyTuple_SetItem( pArgs, 5, PyString_FromString( files.toUtf8().data()));

   if( PyClass::init( AFPYNAMES::SERVICE_CLASSESDIR, name.toUtf8().data(), pArgs) == false) return;

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
      wdir = QString::fromUtf8( PyString_AsString( pResult));
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
      command = QString::fromUtf8( PyString_AsString( pResult));
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
      files = QString::fromUtf8( PyString_AsString( pResult));
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
