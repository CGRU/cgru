#pragma once

#include <Python.h>

#include <string>
#include <vector>

#include "../libafapi/api.h"

typedef struct { PyObject_HEAD afapi::Job   * job;  } PyAf_Job_Object;

typedef struct { PyObject_HEAD afapi::Block * block;} PyAf_Block_Object;

typedef struct { PyObject_HEAD afapi::Task * task;  } PyAf_Task_Object;

typedef struct { PyObject_HEAD afapi::Cmd * cmd;    } PyAf_Cmd_Object;

namespace PyAf
{
   bool GetInteger( PyObject * obj, long long & value, const char * errMsg = NULL);
   bool GetString(  PyObject * obj, std::string & str, const char * errMsg = NULL);
   bool GetStrings( PyObject * obj, std::vector<std::string> & list, int min = -1, int max = -1, const char * errMsg = NULL);
   char * GetData( Py_ssize_t & length, PyObject * obj, const char * errMsg = NULL);
}
