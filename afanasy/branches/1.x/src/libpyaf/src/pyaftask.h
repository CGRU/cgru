#pragma once

#include <Python.h>
#include <structmember.h>

#include <apitask.h>

#include "pyaf.h"

int  PyAf_Task_init(    PyAf_Task_Object * self, PyObject *args, PyObject *kwds);
void PyAf_Task_dealloc( PyAf_Task_Object * self);

PyObject * PyAf_Task_setName(          PyAf_Task_Object *self, PyObject *arg);
PyObject * PyAf_Task_setCommand(       PyAf_Task_Object *self, PyObject *arg);
PyObject * PyAf_Task_setCommandView(   PyAf_Task_Object *self, PyObject *arg);
PyObject * PyAf_Task_setDepend(        PyAf_Task_Object *self, PyObject *arg);
PyObject * PyAf_Task_output(           PyAf_Task_Object *self, PyObject *arg);

#ifndef PYAFTASK
static PyMethodDef PyAf_Task_methods[] = {
   {"setName",          (PyCFunction) PyAf_Task_setName,          METH_O, "Set name."                    },
   {"setCommand",       (PyCFunction) PyAf_Task_setCommand,       METH_O, "Set command."                 },
   {"setCommandView",   (PyCFunction) PyAf_Task_setCommandView,   METH_O, "Set preview command argument."},
   {"setDepend",        (PyCFunction) PyAf_Task_setDepend,        METH_O, "Set depends."                 },
   {"output",           (PyCFunction) PyAf_Task_output,           METH_O, "Print information."           },
   { NULL, NULL, 0, NULL } // Sentinel
};

static PyTypeObject PyAf_Task_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pyaf.Task",             /*tp_name*/
    sizeof(PyAf_Task_Object), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
   (destructor)PyAf_Task_dealloc, /*tp_dealloc*/
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
    PyAf_Task_methods,             /* tp_methods */
    0,                           /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyAf_Task_init,      /* tp_init */
    0,                         /* tp_alloc */
    0,                        /* tp_new */
};
#endif
