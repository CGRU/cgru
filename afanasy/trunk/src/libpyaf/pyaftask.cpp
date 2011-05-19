#define PYAFTASK
#include "pyaftask.h"

int PyAf_Task_init( PyAf_Task_Object *self, PyObject *args, PyObject *kwds)
{
   self->task = new afapi::Task();
   return 0;
}

void PyAf_Task_dealloc( PyAf_Task_Object * self)
{
   delete self->task;
#if PY_MAJOR_VERSION < 3
   self->ob_type->tp_free((PyObject*)self);
#else
   Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject * PyAf_Task_setName( PyAf_Task_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Task_setName")) Py_RETURN_FALSE;
   self->task->setName( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setCommand( PyAf_Task_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Task_setCommand")) Py_RETURN_FALSE;
   self->task->setCommand( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setFiles( PyAf_Task_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Task_setFiles")) Py_RETURN_FALSE;
   self->task->setFiles( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setDependMask( PyAf_Task_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Task_setDependMask")) Py_RETURN_FALSE;
   self->task->setDependMask( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_setCustomData( PyAf_Task_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Task_setCustomData")) Py_RETURN_FALSE;
   self->task->setCustomData( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Task_output( PyAf_Task_Object *self, PyObject *arg)
{
   std::string str;
   if( false == PyAf::GetString(arg, str, "PyAf_Task_output")) Py_RETURN_FALSE;
   self->task->stdOut( PyLong_AsLong( arg));
   Py_RETURN_TRUE;
}
