#pragma once

#include <Python.h>
#include <structmember.h>

#include "../libafapi/apicmd.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "pyaf.h"

int  PyAf_Cmd_init(    PyAf_Cmd_Object * self, PyObject *args);
void PyAf_Cmd_dealloc( PyAf_Cmd_Object * self);

PyObject * PyAf_Cmd_setUserName(             PyAf_Cmd_Object *self,     PyObject *args);
PyObject * PyAf_Cmd_setHostName(             PyAf_Cmd_Object *self,     PyObject *args);
PyObject * PyAf_Cmd_getJobList(              PyAf_Cmd_Object *self,     PyObject *args);
PyObject * PyAf_Cmd_decodeJobList(           PyAf_Cmd_Object *self,     PyObject *args);
PyObject * PyAf_Cmd_getJobInfo(              PyAf_Cmd_Object *self,     PyObject *args);
PyObject * PyAf_Cmd_decodeJobInfo(           PyAf_Cmd_Object *self,     PyObject *args);
PyObject * PyAf_Cmd_deleteJob(               PyAf_Cmd_Object *self, 	PyObject *args);
PyObject * PyAf_Cmd_rendersetnimby(          PyAf_Cmd_Object *self, 	PyObject *arg );
PyObject * PyAf_Cmd_rendersetNIMBY(          PyAf_Cmd_Object *self, 	PyObject *arg );
PyObject * PyAf_Cmd_rendersetfree(           PyAf_Cmd_Object *self, 	PyObject *arg );
PyObject * PyAf_Cmd_renderejecttasks(        PyAf_Cmd_Object *self, 	PyObject *arg );
PyObject * PyAf_Cmd_getDataLen(              PyAf_Cmd_Object *self);
PyObject * PyAf_Cmd_getData(                 PyAf_Cmd_Object *self);

#ifndef PYAFCMD
static PyMethodDef PyAf_Cmd_methods[] = {
   {"setUserName",            (PyCFunction) PyAf_Cmd_setUserName,             METH_O,        "Set action user name."       },
   {"setHostName",            (PyCFunction) PyAf_Cmd_setHostName,             METH_O,        "Set action host name."       },
   {"getjoblist",             (PyCFunction) PyAf_Cmd_getJobList,              METH_VARARGS,  "Get Job List."               },
   {"decodejoblist",          (PyCFunction) PyAf_Cmd_decodeJobList,           METH_VARARGS,  "Decode Job List."            },
   {"getjobinfo",             (PyCFunction) PyAf_Cmd_getJobInfo,              METH_VARARGS,  "Get job info."               },
   {"decodejobinfo",          (PyCFunction) PyAf_Cmd_decodeJobInfo,           METH_VARARGS,  "Decode job info."            },
   {"deletejob",              (PyCFunction) PyAf_Cmd_deleteJob,               METH_VARARGS,  "Delete Job(s)."              },
   {"rendersetnimby",         (PyCFunction) PyAf_Cmd_rendersetnimby,          METH_VARARGS,  "Set render 'nimby'."         },
   {"rendersetNIMBY",         (PyCFunction) PyAf_Cmd_rendersetNIMBY,          METH_VARARGS,  "Set render 'NIMBY'."         },
   {"rendersetfree",          (PyCFunction) PyAf_Cmd_rendersetfree,           METH_VARARGS,  "Free render(s)."             },
   {"renderejecttasks",       (PyCFunction) PyAf_Cmd_renderejecttasks,        METH_VARARGS,  "Eject render running tasks"  },
   {"getDataLen",             (PyCFunction) PyAf_Cmd_getDataLen,              METH_NOARGS,   "Get job data length."        },
   {"getData",                (PyCFunction) PyAf_Cmd_getData,                 METH_NOARGS,   "Get job data."               },
   { NULL, NULL, 0, NULL } // Sentinel
};

#if PY_MAJOR_VERSION >= 3
static PyTypeObject PyAf_Cmd_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyaf.Cmd",             /*tp_name*/
    sizeof(PyAf_Cmd_Object), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
   (destructor)PyAf_Cmd_dealloc, /*tp_dealloc*/
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
    PyAf_Cmd_methods,             /* tp_methods */
    0,                           /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyAf_Cmd_init,      /* tp_init */
    0,                         /* tp_alloc */
    0,                        /* tp_new */
};
#else //PY3K
static PyTypeObject PyAf_Cmd_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pyaf.Cmd",             /*tp_name*/
    sizeof(PyAf_Cmd_Object), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
   (destructor)PyAf_Cmd_dealloc, /*tp_dealloc*/
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
    PyAf_Cmd_methods,             /* tp_methods */
    0,                           /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyAf_Cmd_init,      /* tp_init */
    0,                         /* tp_alloc */
    0,                        /* tp_new */
};
#endif //PY3K
#endif
