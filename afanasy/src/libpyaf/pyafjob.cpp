#define PYAFJOB
#include "pyafjob.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int PyAf_Job_init( PyAf_Job_Object *self, PyObject *args, PyObject *kwds)
{
// Tell job not to delete blocks. Python will free all objects itself.
   self->job = new afapi::Job( false);
   return 0;
}

void PyAf_Job_dealloc( PyAf_Job_Object * self)
{
   delete self->job;
#if PY_MAJOR_VERSION < 3
   self->ob_type->tp_free((PyObject*)self);
#else
   Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject * PyAf_Job_appendBlock( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->appendBlock( ((PyAf_Block_Object*)( arg))->block)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setName( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setName")) Py_RETURN_FALSE;
   self->job->setName( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setDescription( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setDescription")) Py_RETURN_FALSE;
   self->job->setDescription( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setUserName( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setUserName")) Py_RETURN_FALSE;
   self->job->setUserName( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setHostName( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setHostName")) Py_RETURN_FALSE;
   self->job->setHostName( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setCmdPre( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setCmdPre")) Py_RETURN_FALSE;
   self->job->setCmdPre( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setCmdPost( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setCmdPost")) Py_RETURN_FALSE;
   self->job->setCmdPost( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setHostsMask( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setHostsMask")) Py_RETURN_FALSE;
   if( self->job->setHostsMask( str)) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setHostsMaskExclude( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setHostsMaskExclude")) Py_RETURN_FALSE;
   if( self->job->setHostsMaskExclude( str)) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setDependMask( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setDependMask")) Py_RETURN_FALSE;
   if( self->job->setDependMask( str)) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setDependMaskGlobal( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setDependMaskGlobal")) Py_RETURN_FALSE;
   if( self->job->setDependMaskGlobal( str)) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setNeedOS( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setNeedOS")) Py_RETURN_FALSE;
   if( self->job->setNeedOS( str)) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setNeedProperties( PyAf_Job_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Job_setNeedProperties")) Py_RETURN_FALSE;
   if( self->job->setNeedProperties( str)) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setMaxRunningTasks( PyAf_Job_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Job_setMaxRunningTasks")) Py_RETURN_FALSE;
   self->job->setMaxRunningTasks( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setMaxRunTasksPerHost( PyAf_Job_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Job_setMaxRunTasksPerHost")) Py_RETURN_FALSE;
   self->job->setMaxRunTasksPerHost( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setPriority( PyAf_Job_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Job_setPriority")) Py_RETURN_FALSE;
   self->job->setPriority( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setWaitTime( PyAf_Job_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Job_setWaitTime")) Py_RETURN_FALSE;
   self->job->setWaitTime( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_output( PyAf_Job_Object *self, PyObject *arg)
{
   long long value;
   if( false == PyAf::GetInteger(arg, value, "PyAf_Job_output")) Py_RETURN_FALSE;
   self->job->output( value);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_offline( PyAf_Job_Object *self)
{
   self->job->offline();
   Py_RETURN_NONE;
}

PyObject * PyAf_Job_clearBlocksList( PyAf_Job_Object *self)
{
   self->job->clearBlocksList();
   Py_RETURN_NONE;
}

PyObject * PyAf_Job_construct( PyAf_Job_Object *self)
{
   if( self->job->construct()) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_getDataLen( PyAf_Job_Object *self)
{
   PyObject * result = PyLong_FromLong( self->job->getDataLen());
   Py_INCREF( result);
   return result;
}

PyObject * PyAf_Job_getData( PyAf_Job_Object *self)
{
#if PY_MAJOR_VERSION < 3
   PyObject * result = PyString_FromStringAndSize( self->job->getData(), self->job->getDataLen());
#else
   PyObject * result = PyBytes_FromStringAndSize( self->job->getData(), self->job->getDataLen());
#endif
   Py_INCREF( result);
   return result;
}
