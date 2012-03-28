#pragma once

//#include <Python.h>
//#include <structmember.h>

#include "../libafapi/apiblock.h"

#include "pyaf.h"

int  PyAf_Block_init(    PyAf_Block_Object * self, PyObject *args, PyObject *kwds);
void PyAf_Block_dealloc( PyAf_Block_Object * self);

PyObject * PyAf_Block_appendTask(            PyAf_Block_Object *self, PyObject *arg);                                                                                                                                                              
PyObject * PyAf_Block_clearTasksList(        PyAf_Block_Object *self);                                                                                                                                                                             
PyObject * PyAf_Block_output(                PyAf_Block_Object *self, PyObject *arg);                                                                                                                                                              
PyObject * PyAf_Block_setCapacity(           PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCmdPost(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCmdPre(             PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCommand(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setCustomData(         PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setDependMask(         PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setDependSubTask(      PyAf_Block_Object *self);
PyObject * PyAf_Block_setEnvironment(        PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setErrorsAvoidHost(    PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setErrorsForgiveTime(  PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setErrorsRetries(      PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setErrorsTaskSameHost( PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setFileSizeCheck(      PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setFiles(              PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setFramesPerTask(      PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setHostsMask(          PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setHostsMaskExclude(   PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setMaxRunTasksPerHost( PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setMaxRunningTasks(    PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setMultiHost(          PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setName(               PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedHDD(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedMemory(         PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedPower(          PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNeedProperties(     PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setNonSequential(      PyAf_Block_Object *self);
PyObject * PyAf_Block_setNumeric(            PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setParser(             PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setParserCoeff(        PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setService(            PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setTasksDependMask(    PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setTasksMaxRunTime(    PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setTasksName(          PyAf_Block_Object *self, PyObject *arg);
PyObject * PyAf_Block_setVariableCapacity(   PyAf_Block_Object *self, PyObject *args);
PyObject * PyAf_Block_setWorkingDirectory(   PyAf_Block_Object *self, PyObject *arg);

#ifndef PYAFBLOCK
static PyMethodDef PyAf_Block_methods[] = {
   {"appendTask",             (PyCFunction) PyAf_Block_appendTask,            METH_O,        "Append task."},
   {"clearTasksList",         (PyCFunction) PyAf_Block_clearTasksList,        METH_NOARGS,   "Clear tasks list."},
   {"output",                 (PyCFunction) PyAf_Block_output,                METH_O,        "Print information."},
   {"setCapacity",            (PyCFunction) PyAf_Block_setCapacity,           METH_O,        "Set capacity."},
   {"setCmdPost",             (PyCFunction) PyAf_Block_setCmdPost,            METH_O,        "Set post command."},
   {"setCmdPre",              (PyCFunction) PyAf_Block_setCmdPre,             METH_O,        "Set pre command."},
   {"setCommand",             (PyCFunction) PyAf_Block_setCommand,            METH_O,        "Set tasks command."},
   {"setCustomData",          (PyCFunction) PyAf_Block_setCustomData,         METH_O,        "Set custom data."},
   {"setDependMask",          (PyCFunction) PyAf_Block_setDependMask,         METH_O,        "Set depend mask."},
   {"setDependSubTask",       (PyCFunction) PyAf_Block_setDependSubTask,      METH_NOARGS,   "Set sub task block dependence."},
   {"setEnvironment",         (PyCFunction) PyAf_Block_setEnvironment,        METH_O,        "Set tasks extra environment."},
   {"setErrorsAvoidHost",     (PyCFunction) PyAf_Block_setErrorsAvoidHost,    METH_O,        "Set maximum number or errors on same host for job NOT to avoid host."},
   {"setErrorsForgiveTime",   (PyCFunction) PyAf_Block_setErrorsForgiveTime,  METH_O,        "Set time to forgive error host."},
   {"setErrorsRetries",       (PyCFunction) PyAf_Block_setErrorsRetries,      METH_O,        "Set maximum number of errors in task to retry it automatically."},
   {"setErrorsTaskSameHost",  (PyCFunction) PyAf_Block_setErrorsTaskSameHost, METH_O,        "Set maximum number or errors on same host for task NOT to avoid host."},
   {"setFileSizeCheck",       (PyCFunction) PyAf_Block_setFileSizeCheck,      METH_VARARGS,  "Set file size check."},
   {"setFiles",               (PyCFunction) PyAf_Block_setFiles,              METH_O,        "Set files."},
   {"setFramesPerTask",       (PyCFunction) PyAf_Block_setFramesPerTask,      METH_O,        "Set frames fer task."},
   {"setHostsMask",           (PyCFunction) PyAf_Block_setHostsMask,          METH_O,        "Set hosts mask."},
   {"setHostsMaskExclude",    (PyCFunction) PyAf_Block_setHostsMaskExclude,   METH_O,        "Set exclude hosts mask."},
   {"setMaxRunTasksPerHost",  (PyCFunction) PyAf_Block_setMaxRunTasksPerHost, METH_O,        "Set maximum running tasks per host."},
   {"setMaxRunningTasks",     (PyCFunction) PyAf_Block_setMaxRunningTasks,    METH_O,        "Set maximum running tasks."},
   {"setMultiHost",           (PyCFunction) PyAf_Block_setMultiHost,          METH_VARARGS,  "Set multi hosts tasks."},
   {"setName",                (PyCFunction) PyAf_Block_setName,               METH_O,        "Set name."},
   {"setNeedHDD",             (PyCFunction) PyAf_Block_setNeedHDD,            METH_O,        "Set HDD needed."},
   {"setNeedMemory",          (PyCFunction) PyAf_Block_setNeedMemory,         METH_O,        "Set memory needed."},
   {"setNeedPower",           (PyCFunction) PyAf_Block_setNeedPower,          METH_O,        "Set power needed."},
   {"setNeedProperties",      (PyCFunction) PyAf_Block_setNeedProperties,     METH_O,        "Set needed properties."},
   {"setNonSequential",       (PyCFunction) PyAf_Block_setNonSequential,      METH_NOARGS,   "Set non-seqential tasks running."},
   {"setNumeric",             (PyCFunction) PyAf_Block_setNumeric,            METH_VARARGS,  "Set block type to numeric."},
   {"setParser",              (PyCFunction) PyAf_Block_setParser,             METH_O,        "Set tasks parser type."},
   {"setParserCoeff",         (PyCFunction) PyAf_Block_setParserCoeff,        METH_O,        "Set tasks parser coefficient."},
   {"setService",             (PyCFunction) PyAf_Block_setService,            METH_O,        "Set tasks service type."},
   {"setTasksDependMask",     (PyCFunction) PyAf_Block_setTasksDependMask,    METH_O,        "Set tasks depend mask."},
   {"setTasksMaxRunTime",     (PyCFunction) PyAf_Block_setTasksMaxRunTime,    METH_O,        "Set tasks maximum run time."},
   {"setTasksName",           (PyCFunction) PyAf_Block_setTasksName,          METH_O,        "Set tasks name."},
   {"setVariableCapacity",    (PyCFunction) PyAf_Block_setVariableCapacity,   METH_VARARGS,  "Set variable capacity."},
   {"setWorkingDirectory",    (PyCFunction) PyAf_Block_setWorkingDirectory,   METH_O,        "Set tasks working directory."},
   { NULL, NULL, 0, NULL } // Sentinel
};

static PyTypeObject PyAf_Block_Type = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
#endif
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
