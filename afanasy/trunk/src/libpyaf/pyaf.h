#pragma once

#include <Python.h>

#include "../libafapi/api.h"

typedef struct { PyObject_HEAD afapi::Job   * job;  } PyAf_Job_Object;

typedef struct { PyObject_HEAD afapi::Block * block;} PyAf_Block_Object;

typedef struct { PyObject_HEAD afapi::Task * task;  } PyAf_Task_Object;

typedef struct { PyObject_HEAD afapi::Cmd * cmd;    } PyAf_Cmd_Object;

namespace PyAf
{
}
