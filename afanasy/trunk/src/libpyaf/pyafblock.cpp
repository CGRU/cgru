#define PYAFBLOCK
#include "pyafblock.h"

#define _DEBUG
#undef _DEBUG
#include "../include/macrooutput.h"

int PyAf_Block_init( PyAf_Block_Object *self, PyObject *args, PyObject *kwds)
{
   self->block = new afapi::Block();
   return 0;
}

void PyAf_Block_dealloc( PyAf_Block_Object * self)
{
   delete self->block;
   self->ob_type->tp_free((PyObject*)self);
}

PyObject * PyAf_Block_appendTask( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->appendTask( ((PyAf_Task_Object*)( arg))->task);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setName( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setName( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setTasksName( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setTasksName( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setService( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setService( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setParser( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setParser( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setWorkingDirectory( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setWorkingDirectory( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setEnvironment( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setEnvironment( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCommand( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setCommand( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setFiles( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setFiles( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCustomData( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setCustomData( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCmdPre( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setCmdPre( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCmdPost( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setCmdPost( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setHostsMask( PyAf_Block_Object *self, PyObject *arg)
{
   if( self->block->setHostsMask( PyString_AsString( arg))) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setHostsMaskExclude( PyAf_Block_Object *self, PyObject *arg)
{
   if( self->block->setHostsMaskExclude( PyString_AsString( arg))) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setDependMask( PyAf_Block_Object *self, PyObject *arg)
{
   if( self->block->setDependMask( PyString_AsString( arg))) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setTasksDependMask( PyAf_Block_Object *self, PyObject *arg)
{
   if( self->block->setTasksDependMask( PyString_AsString( arg))) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setNeedProperties( PyAf_Block_Object *self, PyObject *arg)
{
   if( self->block->setNeedProperties( PyString_AsString( arg))) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setCapacity( PyAf_Block_Object *self, PyObject *arg)
{
   if( self->block->setCapacity( PyInt_AsLong( arg))) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setMaxRunningTasks( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setMaxRunningTasks( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setTasksMaxRunTime( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setTasksMaxRunTime( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNeedMemory( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setNeedMemory( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNeedPower( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setNeedPower( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNeedHDD( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setNeedHDD( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setParserCoeff( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setParserCoeff( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setFramesPerTask( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->setFramesPerTask( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setFileSizeCheck( PyAf_Block_Object *self, PyObject *args)
{
   int min, max;
   if( false == PyArg_ParseTuple( args, "ii", &min, &max)) Py_RETURN_FALSE;
   self->block->setFileSizeCheck( min, max);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setVariableCapacity( PyAf_Block_Object *self, PyObject *args)
{
   int min, max;
   if( false == PyArg_ParseTuple( args, "ii", &min, &max)) Py_RETURN_FALSE;
   self->block->setVariableCapacity( min, max);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setMultiHost( PyAf_Block_Object *self, PyObject *args)
{
   int min, max, waitmax, sameHostMaster, waitsrv;
   const char * service;
   if( false == PyArg_ParseTuple( args, "iiiisi", &min, &max, &waitmax, &sameHostMaster, &service, &waitsrv)) Py_RETURN_FALSE;
   self->block->setMultiHost( min, max, waitmax, sameHostMaster, service, waitsrv);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNumeric( PyAf_Block_Object *self, PyObject *args)
{
   int start, end, perHost, incement;
   if( false == PyArg_ParseTuple( args, "iiii", &start, &end, &perHost, &incement)) Py_RETURN_FALSE;
   if( self->block->setNumeric( start, end, perHost, incement)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_clearTasksList( PyAf_Block_Object *self)
{
   self->block->clearTasksList();
   Py_RETURN_NONE;
}

PyObject * PyAf_Block_output( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->stdOut( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}
