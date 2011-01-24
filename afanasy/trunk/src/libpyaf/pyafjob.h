#pragma once

#include <Python.h>
#include <structmember.h>

#include "../libafapi/apijob.h"

#include "pyaf.h"

int  PyAf_Job_init(    PyAf_Job_Object * self, PyObject *args, PyObject *kwds);
void PyAf_Job_dealloc( PyAf_Job_Object * self);

PyObject * PyAf_Job_appendBlock(          PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setName(              PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setDescription(       PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setUserName(          PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setHostName(          PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setHostsMask(         PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setHostsMaskExclude(  PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setDependMask(        PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setDependMaskGlobal(  PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setNeedOS(            PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setNeedProperties(    PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setCmdPre(            PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setCmdPost(           PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setMaxRunningTasks(   PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setPriority(          PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_setWaitTime(          PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_output(               PyAf_Job_Object *self, PyObject *arg);
PyObject * PyAf_Job_offline(              PyAf_Job_Object *self);
PyObject * PyAf_Job_clearBlocksList(      PyAf_Job_Object *self);
PyObject * PyAf_Job_construct(            PyAf_Job_Object *self);
PyObject * PyAf_Job_getDataLen(           PyAf_Job_Object *self);
PyObject * PyAf_Job_getData(              PyAf_Job_Object *self);

#ifndef PYAFJOB
static PyMethodDef PyAf_Job_methods[] = {
   {"appendBlock",         (PyCFunction) PyAf_Job_appendBlock,          METH_O,        "Append block."               },
   {"setName",             (PyCFunction) PyAf_Job_setName,              METH_O,        "Set name."                   },
   {"setDescription",      (PyCFunction) PyAf_Job_setDescription,       METH_O,        "Set description."            },
   {"setUserName",         (PyCFunction) PyAf_Job_setUserName,          METH_O,        "Set user name."              },
   {"setHostName",         (PyCFunction) PyAf_Job_setHostName,          METH_O,        "Set host name."              },
   {"setHostsMask",        (PyCFunction) PyAf_Job_setHostsMask,         METH_O,        "Set hosts mask."             },
   {"setHostsMaskExclude", (PyCFunction) PyAf_Job_setHostsMaskExclude,  METH_O,        "Set exclude hosts mask."     },
   {"setDependMask",       (PyCFunction) PyAf_Job_setDependMask,        METH_O,        "Set depend mask."            },
   {"setDependMaskGlobal", (PyCFunction) PyAf_Job_setDependMaskGlobal,  METH_O,        "Set global depend mask."     },
   {"setNeedOS",           (PyCFunction) PyAf_Job_setNeedOS,            METH_O,        "Set need os."                },
   {"setNeedProperties",   (PyCFunction) PyAf_Job_setNeedProperties,    METH_O,        "Set need properties."        },
   {"setCmdPre",           (PyCFunction) PyAf_Job_setCmdPre,            METH_O,        "Set pre command."            },
   {"setCmdPost",          (PyCFunction) PyAf_Job_setCmdPost,           METH_O,        "Set post command."           },
   {"setMaxRunningTasks",  (PyCFunction) PyAf_Job_setMaxRunningTasks,   METH_O,        "Set maximum running tasks."  },
   {"setPriority",         (PyCFunction) PyAf_Job_setPriority,          METH_O,        "Set priority."               },
   {"setWaitTime",         (PyCFunction) PyAf_Job_setWaitTime,          METH_O,        "Set wait time."              },
   {"output",              (PyCFunction) PyAf_Job_output,               METH_O,        "Print information."          },
   {"offline",             (PyCFunction) PyAf_Job_offline,              METH_NOARGS,   "Start job paused."           },
   {"clearBlocksList",     (PyCFunction) PyAf_Job_clearBlocksList,      METH_NOARGS,   "Clear blocks list."          },
   {"construct",           (PyCFunction) PyAf_Job_construct,            METH_NOARGS,   "Construct job data."         },
   {"getDataLen",          (PyCFunction) PyAf_Job_getDataLen,           METH_NOARGS,   "Get job data length."        },
   {"getData",             (PyCFunction) PyAf_Job_getData,              METH_NOARGS,   "Get job data."               },
   { NULL, NULL, 0, NULL } // Sentinel
};

static PyTypeObject PyAf_Job_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pyaf.Job",             /*tp_name*/
    sizeof(PyAf_Job_Object), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
   (destructor)PyAf_Job_dealloc, /*tp_dealloc*/
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
    "Afanasy Job",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyAf_Job_methods,             /* tp_methods */
    0,                           /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyAf_Job_init,      /* tp_init */
    0,                         /* tp_alloc */
    0,                        /* tp_new */
};
#endif
