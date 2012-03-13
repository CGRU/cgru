#include "service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "services.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern char PYNAME_FuncApplyCmdCapacity[];
extern char PYNAME_FuncApplyCmdHosts[];
extern char PYNAME_FuncCheckFiles[];

Service::Service( const QString & type,
            const QString & command,
            const QString & files,
            bool verbose):
   name( type),
   PyObj_ServiceClass( NULL),
   PyObj_Instance( NULL),
   PyObj_FuncApplyCmdCapacity( NULL),
   PyObj_FuncApplyCmdHosts( NULL),
   PyObj_FuncCheckFiles( NULL),
   initialized( false)
{
   PyObj_ServiceClass = srv()->getServiceClass( type);
   if( PyObj_ServiceClass == NULL) return;

   PyObject *pArgs;
   pArgs = PyTuple_New( 2);
   PyTuple_SetItem( pArgs, 0, PyString_FromString( command.toUtf8().data() ));
   PyTuple_SetItem( pArgs, 1, PyString_FromString(   files.toUtf8().data() ));
   PyObj_Instance = PyInstance_New( PyObj_ServiceClass, pArgs, NULL);
   Py_DECREF( pArgs);
   if( PyObj_Instance == NULL)
   {
      PyErr_Print();
      AFERRAR( "Failed to instance \"%s\"\n", name.toUtf8().data());
      return;
   }
   PyObj_FuncApplyCmdCapacity = PyObject_GetAttrString( PyObj_Instance, PYNAME_FuncApplyCmdCapacity);
   PyObj_FuncApplyCmdHosts    = PyObject_GetAttrString( PyObj_Instance, PYNAME_FuncApplyCmdHosts);
   PyObj_FuncCheckFiles       = PyObject_GetAttrString( PyObj_Instance, PYNAME_FuncCheckFiles);

   initialized = true;
}

Service::~Service()
{
   if( PyObj_Instance) Py_XDECREF( PyObj_Instance);
}

const QString Service::applyCmdCapacity( int capacity)
{
   QString newcomand;
   PyObject * pArgs = PyTuple_New( 1);
   PyTuple_SetItem( pArgs, 0, PyInt_FromLong( capacity));
   PyObject * pResult = PyObject_CallObject( PyObj_FuncApplyCmdCapacity, pArgs);
   Py_DECREF( pArgs);

   if( PyString_Check( pResult))
      newcomand = PyString_AsString( pResult);
   else
      AFERROR("Service::applyCmdCapacity: returned object is not a string\n");

   Py_DECREF( pResult);
   return newcomand;
}

const QString Service::applyCmdHosts( const QStringList & hosts)
{
   QString newcomand;

   PyObject * pList = PyList_New(0);
   for( int h = 0; h < hosts.size(); h++)
      if( PyList_Append( pList, PyString_FromString( hosts[h].toUtf8().data())) != 0)
      {
         AFERROR("Service::applyCmdHosts: PyList_Append:\n");
         PyErr_Print();
         return newcomand;
      }

   PyObject * pArgs = PyTuple_New( 1);
   if( PyTuple_SetItem( pArgs, 0, pList) != 0)
   {
      AFERROR("Service::applyCmdHosts: PyTuple_SetItem:\n");
      PyErr_Print();
      return newcomand;
   }

   PyObject * pResult = PyObject_CallObject( PyObj_FuncApplyCmdHosts, pArgs);
   Py_DECREF( pArgs);

   if( PyString_Check( pResult))
      newcomand = PyString_AsString( pResult);
   else
      AFERROR("Service::applyCmdHosts: returned object is not a string\n");

   return newcomand;
}

bool Service::checkFiles( int sizemin, int sizemax)
{
   return true;
}
