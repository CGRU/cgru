#define PYAFBLOCK
#include "pyafblock.h"

#define _DEBUG
#undef _DEBUG
#include "../include/macrooutput.h"

int PyAf_Block_init( PyAf_Block_Object *self, PyObject *args, PyObject *kwds)
{
// Tell block not to delete tasks. Python will free all objects itself.
   self->block = new afapi::Block( false);
   return 0;
}

void PyAf_Block_dealloc( PyAf_Block_Object * self)
{
   delete self->block;
#if PY_MAJOR_VERSION < 3
   self->ob_type->tp_free((PyObject*)self);
#else
   Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject * PyAf_Block_appendTask( PyAf_Block_Object *self, PyObject *arg)
{
   self->block->appendTask( ((PyAf_Task_Object*)( arg))->task);
   Py_RETURN_TRUE;
}


PyObject * PyAf_Block_clearTasksList( PyAf_Block_Object *self)
{
   self->block->clearTasksList();
   Py_RETURN_NONE;
}


PyObject * PyAf_Block_output( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_output")) Py_RETURN_FALSE;
   self->block->stdOut( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCapacity( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setCapacity")) Py_RETURN_FALSE;
   if( self->block->setCapacity( value)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setCmdPost( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setCmdPost")) Py_RETURN_FALSE;
   self->block->setCmdPost( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCmdPre( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setCmdPre")) Py_RETURN_FALSE;
   self->block->setCmdPre( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCommand( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setCommand")) Py_RETURN_FALSE;
   self->block->setCommand( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setCustomData( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setCustomData")) Py_RETURN_FALSE;
   self->block->setCustomData( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setDependMask( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setDependMask")) Py_RETURN_FALSE;
   if( self->block->setDependMask( str)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setDependSubTask( PyAf_Block_Object *self)
{
   self->block->setDependSubTask( true);
   Py_RETURN_NONE;
}

PyObject * PyAf_Block_setEnvironment( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setEnvironment")) Py_RETURN_FALSE;
   self->block->setEnvironment( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setErrorsAvoidHost( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setErrorsAvoidHost")) Py_RETURN_FALSE;
   self->block->setErrorsAvoidHost( value);
   Py_RETURN_NONE;
}

PyObject * PyAf_Block_setErrorsForgiveTime( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setErrorsForgiveTime")) Py_RETURN_FALSE;
   self->block->setErrorsForgiveTime( value);
   Py_RETURN_NONE;
}

PyObject * PyAf_Block_setErrorsRetries( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setErrorsRetries")) Py_RETURN_FALSE;
   self->block->setErrorsRetries( value);
   Py_RETURN_NONE;
}

PyObject * PyAf_Block_setErrorsTaskSameHost( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setErrorsTaskSameHost")) Py_RETURN_FALSE;
   self->block->setErrorsTaskSameHost( value);
   Py_RETURN_NONE;
}

PyObject * PyAf_Block_setFileSizeCheck( PyAf_Block_Object *self, PyObject *args)
{
   long long min, max;
   if( false == PyArg_ParseTuple( args, "LL", &min, &max)) Py_RETURN_FALSE;
   self->block->setFileSizeCheck( min, max);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setFiles( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setFiles")) Py_RETURN_FALSE;
   self->block->setFiles( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setFramesPerTask( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setFramesPerTask")) Py_RETURN_FALSE;
   self->block->setFramesPerTask( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setHostsMask( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setHostsMask")) Py_RETURN_FALSE;
   if( self->block->setHostsMask( str)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setHostsMaskExclude( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setHostsMaskExclude")) Py_RETURN_FALSE;
   if( self->block->setHostsMaskExclude( str)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setMaxRunTasksPerHost( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setMaxRunTasksPerHost")) Py_RETURN_FALSE;
   self->block->setMaxRunTasksPerHost( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setMaxRunningTasks( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setMaxRunningTasks")) Py_RETURN_FALSE;
   self->block->setMaxRunningTasks( value);
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

PyObject * PyAf_Block_setName( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setName")) Py_RETURN_FALSE;
   self->block->setName( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNeedHDD( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setNeedHDD")) Py_RETURN_FALSE;
   self->block->setNeedHDD( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNeedMemory( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setNeedMemory")) Py_RETURN_FALSE;
   self->block->setNeedMemory( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNeedPower( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setNeedPower")) Py_RETURN_FALSE;
   self->block->setNeedPower( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setNeedProperties( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setNeedProperties")) Py_RETURN_FALSE;
   if( self->block->setNeedProperties( str)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setNumeric( PyAf_Block_Object *self, PyObject *args)
{
   long long start, end, perHost, incement;
   if( false == PyArg_ParseTuple( args, "LLLL", &start, &end, &perHost, &incement)) Py_RETURN_FALSE;
   if( self->block->setNumeric( start, end, perHost, incement)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setParser( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setParser")) Py_RETURN_FALSE;
   self->block->setParser( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setParserCoeff( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setParserCoeff")) Py_RETURN_FALSE;
   self->block->setParserCoeff( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setService( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setService")) Py_RETURN_FALSE;
   self->block->setService( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setTasksDependMask( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setTasksDependMask")) Py_RETURN_FALSE;
   if( self->block->setTasksDependMask( str)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Block_setTasksMaxRunTime( PyAf_Block_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Block_setTasksMaxRunTime")) Py_RETURN_FALSE;
   self->block->setTasksMaxRunTime( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setTasksName( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setTasksName")) Py_RETURN_FALSE;
   self->block->setTasksName( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setVariableCapacity( PyAf_Block_Object *self, PyObject *args)
{
   int min, max;
   if( false == PyArg_ParseTuple( args, "ii", &min, &max)) Py_RETURN_FALSE;
   self->block->setVariableCapacity( min, max);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Block_setWorkingDirectory( PyAf_Block_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Block_setWorkingDirectory")) Py_RETURN_FALSE;
   self->block->setWorkingDirectory( str);
   Py_RETURN_TRUE;
}