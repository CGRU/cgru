#pragma once

#include <Python.h>
#include <structmember.h>

#include <apiblock.h>

#include "pyaf.h"

int  PyAf_Block_init(    PyAf_Block_Object * self, PyObject *args, PyObject *kwds);
void PyAf_Block_dealloc( PyAf_Block_Object * self);

PyObject * PyAf_Block_appendTask(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_output(                PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setName(               PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setTasksName(          PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setTasksType(          PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setParserType(         PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setWorkingDirectory(   PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setEnvironment(        PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCommand(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCommandView(        PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCmdPre(             PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCmdPost(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setHostsMask(          PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setHostsMaskExclude(   PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setDependMask(         PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setTasksDependMask(    PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedProperties(     PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setMaxHosts(           PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCapacity(           PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setTasksMaxRunTime(    PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedMemory(         PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedPower(          PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedHDD(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setFileSizeCheck(      PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setVariableCapacity(   PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setMultiHost(          PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setNumeric(            PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setFramesPerHost(      PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_clearTasksList(        PyAf_Block_Object *self);

#ifndef PYAFBLOCK
static PyMethodDef PyAf_Block_methods[] = {
   {"appendTask",          (PyCFunction) PyAf_Block_appendTask,            METH_O,        "Append task."                   },
   {"output",              (PyCFunction) PyAf_Block_output,                METH_O,        "Print information."             },
   {"setName",             (PyCFunction) PyAf_Block_setName,               METH_O,        "Set name."                      },
   {"setTasksName",        (PyCFunction) PyAf_Block_setTasksName,          METH_O,        "Set tasks name."                },
   {"setTasksType",        (PyCFunction) PyAf_Block_setTasksType,          METH_O,        "Set tasks service type."        },
   {"setParserType",       (PyCFunction) PyAf_Block_setParserType,         METH_O,        "Set tasks parser type."         },
   {"setWorkingDirectory", (PyCFunction) PyAf_Block_setWorkingDirectory,   METH_O,        "Set tasks working directory."   },
   {"setEnvironment",      (PyCFunction) PyAf_Block_setEnvironment,        METH_O,        "Set tasks extra environment."   },
   {"setCommand",          (PyCFunction) PyAf_Block_setCommand,            METH_O,        "Set tasks command."             },
   {"setCommandView",      (PyCFunction) PyAf_Block_setCommandView,        METH_O,        "Set preview command argument."  },
   {"setCmdPre",           (PyCFunction) PyAf_Block_setCmdPre,             METH_O,        "Set pre command."               },
   {"setCmdPost",          (PyCFunction) PyAf_Block_setCmdPost,            METH_O,        "Set post command."              },
   {"setHostsMask",        (PyCFunction) PyAf_Block_setHostsMask,          METH_O,        "Set hosts mask."                },
   {"setHostsMaskExclude", (PyCFunction) PyAf_Block_setHostsMaskExclude,   METH_O,        "Set exclude hosts mask."        },
   {"setDependMask",       (PyCFunction) PyAf_Block_setDependMask,         METH_O,        "Set depend mask."               },
   {"setTasksDependMask",  (PyCFunction) PyAf_Block_setTasksDependMask,    METH_O,        "Set global depend mask."        },
   {"setNeedProperties",   (PyCFunction) PyAf_Block_setNeedProperties,     METH_O,        "Set needed properties."         },
   {"setMaxHosts",         (PyCFunction) PyAf_Block_setMaxHosts,           METH_O,        "Set maximum hosts."             },
   {"setCapacity",         (PyCFunction) PyAf_Block_setCapacity,           METH_O,        "Set capacity."                  },
   {"setTasksMaxRunTime",  (PyCFunction) PyAf_Block_setTasksMaxRunTime,    METH_O,        "Set tasks maximum run time."    },
   {"setNeedMemory",       (PyCFunction) PyAf_Block_setNeedMemory,         METH_O,        "Set memory needed."             },
   {"setNeedPower",        (PyCFunction) PyAf_Block_setNeedPower,          METH_O,        "Set power needed."              },
   {"setNeedHDD",          (PyCFunction) PyAf_Block_setNeedHDD,            METH_O,        "Set HDD needed."                },
   {"setFileSizeCheck",    (PyCFunction) PyAf_Block_setFileSizeCheck,      METH_VARARGS,  "Set file size check."           },
   {"setVariableCapacity", (PyCFunction) PyAf_Block_setVariableCapacity,   METH_VARARGS,  "Set variable capacity."         },
   {"setMultiHost",        (PyCFunction) PyAf_Block_setMultiHost,          METH_VARARGS,  "Set multi hosts tasks."         },
   {"setNumeric",          (PyCFunction) PyAf_Block_setNumeric,            METH_VARARGS,  "Set block type to numeric."     },
   {"setFramesPerHost",    (PyCFunction) PyAf_Block_setFramesPerHost,      METH_VARARGS,  "Set frames fer host."           },
   {"clearTasksList",      (PyCFunction) PyAf_Block_clearTasksList,        METH_NOARGS,   "Set frames fer host."           },
   { NULL, NULL, 0, NULL } // Sentinel
};

static PyTypeObject PyAf_Block_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pyaf.Block",             /*tp_name*/
    sizeof(PyAf_Block_Object), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
   (destructor)PyAf_Block_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Node objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyAf_Block_methods,             /* tp_methods */
    0,                           /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyAf_Block_init,      /* tp_init */
    0,                         /* tp_alloc */
    0,                        /* tp_new */
};
#endif
