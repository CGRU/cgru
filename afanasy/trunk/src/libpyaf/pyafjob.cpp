#define PYAFJOB
#include "pyafjob.h"

#define _DEBUG
#undef _DEBUG
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
   self->ob_type->tp_free((PyObject*)self);
}

PyObject * PyAf_Job_appendBlock( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->appendBlock( ((PyAf_Block_Object*)( arg))->block)) Py_RETURN_TRUE;
   Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setName( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setName( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setDescription( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setDescription( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setUserName( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setUserName( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setHostName( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setHostName( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setCmdPre( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setCmdPre( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setCmdPost( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setCmdPost( PyString_AsString( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setHostsMask( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->setHostsMask( PyString_AsString( arg))) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setHostsMaskExclude( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->setHostsMaskExclude( PyString_AsString( arg))) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setDependMask( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->setDependMask( PyString_AsString( arg))) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setDependMaskGlobal( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->setDependMaskGlobal( PyString_AsString( arg))) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setNeedOS( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->setNeedOS( PyString_AsString( arg))) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setNeedProperties( PyAf_Job_Object *self, PyObject *arg)
{
   if( self->job->setNeedProperties( PyString_AsString( arg))) Py_RETURN_TRUE;
   else Py_RETURN_FALSE;
}

PyObject * PyAf_Job_setMaxRunningTasks( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setMaxRunningTasks( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setPriority( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setPriority( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_setWaitTime( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->setWaitTime( PyInt_AsLong( arg));
   Py_RETURN_TRUE;
}

PyObject * PyAf_Job_output( PyAf_Job_Object *self, PyObject *arg)
{
   self->job->output( PyInt_AsLong( arg));
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
   PyObject * result = PyInt_FromLong( self->job->getDataLen());
   Py_INCREF( result);
   return result;
}

PyObject * PyAf_Job_getData( PyAf_Job_Object *self)
{
   PyObject * result = PyString_FromStringAndSize( self->job->getData(), self->job->getDataLen());
   Py_INCREF( result);
   return result;
}
