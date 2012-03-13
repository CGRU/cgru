#include "jobpy.h"

#include "../include/afpynames.h"

#include "jobaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

JobPy::JobPy( JobAf * jobPtr):
   initialized( false),
   job( jobPtr),
   PyObj_FuncConstuct( NULL),
   PyObj_FuncAddBlock( NULL),
   PyObj_FuncDestroy( NULL)
{
   PyObject * args = PyTuple_New( 2);

   // Job name
   PyTuple_SetItem( args, 0, PyString_FromString( job->getName().toUtf8().data()));
   // User name
   PyTuple_SetItem( args, 1, PyString_FromString( job->getUserName().toUtf8().data()));

   if( PyClass::init( AFPYNAMES::JOB_CLASSESDIR, AFPYNAMES::JOB_DEFAULTCLASS, args) == false) return;

   // Get functions:
   PyObj_FuncConstuct = getFunction( AFPYNAMES::JOB_FUNC_CONSTRUCT);
   if( PyObj_FuncConstuct == NULL ) return;
   PyObj_FuncAddBlock = getFunction( AFPYNAMES::JOB_FUNC_ADDBLOCK);
   if( PyObj_FuncAddBlock == NULL ) return;
   PyObj_FuncDestroy = getFunction( AFPYNAMES::JOB_FUNC_DESTOY);
   if( PyObj_FuncDestroy == NULL ) return;

   // Adding blocks:
   for( int i = 0; i < job->getBlocksNum(); i ++)
   {
      af::BlockData * block = job->getBlock(i);
      PyObject * args = PyTuple_New( 3);

      // Block name
      PyTuple_SetItem( args, 0, PyString_FromString( block->getName().toUtf8().data()));
      // Tasks type
      PyTuple_SetItem( args, 1, PyString_FromString( block->getTasksType().toUtf8().data()));
      // Tasks number
      PyTuple_SetItem( args, 2, PyInt_FromLong( block->getTasksNum()));

      PyObject * result = PyObject_CallObject( PyObj_FuncAddBlock, args);
      if( PyErr_Occurred()) PyErr_Print();
      Py_XDECREF( args);
      Py_XDECREF( result);
   }

   // Call construct function and check result:
   PyObject * result = PyObject_CallObject( PyObj_FuncConstuct, NULL);
   if( PyErr_Occurred()) PyErr_Print();
   initialized = PyObject_IsTrue( result);
   Py_XDECREF( result);
}

JobPy::~JobPy()
{
   PyObject * result = PyObject_CallObject( PyObj_FuncDestroy, NULL);
   if( PyErr_Occurred()) PyErr_Print();
   Py_XDECREF( result);
}
