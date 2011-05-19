#pragma once

#include <Python.h>

#include <string>

#include "../libafapi/api.h"

#if PY_MAJOR_VERSION < 3
#define PyBytes_Check PyString_Check
#define PyBytes_AsString PyString_AsString
#define PyLong_AsLong PyInt_AsLong
#define PyLong_FromLong PyInt_FromLong
#endif

typedef struct { PyObject_HEAD afapi::Job   * job;  } PyAf_Job_Object;

typedef struct { PyObject_HEAD afapi::Block * block;} PyAf_Block_Object;

typedef struct { PyObject_HEAD afapi::Task * task;  } PyAf_Task_Object;

typedef struct { PyObject_HEAD afapi::Cmd * cmd;    } PyAf_Cmd_Object;

namespace PyAf
{
   bool GetString( PyObject * obj, std::string & str, const char * errMsg = NULL);
}
