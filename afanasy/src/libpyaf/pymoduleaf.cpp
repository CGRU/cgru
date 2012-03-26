#include "pyafjob.h"

#include "pyafblock.h"
#include "pyaftask.h"
#include "pyafcmd.h"

#if PY_MAJOR_VERSION < 3
static PyMethodDef pyaf_Methods[] = {
   { 0, 0, 0, 0 } // Sentinel
};
#else
struct module_state {
   PyObject *error;
};
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
static PyObject * error_out( PyObject *m)
{
   struct module_state *st = GETSTATE(m);
   PyErr_SetString(st->error, "something bad happened");
   return NULL;
}
static PyMethodDef pyaf_Methods[] = {
   {"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
   {NULL, NULL, 0, NULL} // Sentinel
};
static int pyaf_traverse(PyObject *m, visitproc visit, void *arg)
{
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}
static int pyaf_clear(PyObject *m)
{
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}
static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "pyaf",
        NULL,
        sizeof(struct module_state),
        pyaf_Methods,
        NULL,
        pyaf_traverse,
        pyaf_clear,
        NULL
};
#endif

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

#if PY_MAJOR_VERSION < 3
#define RETVAR
PyMODINIT_FUNC initpyaf(void)
#else
#define RETVAR module
PyMODINIT_FUNC PyInit_pyaf(void)
#endif
{
   PyObject * module = NULL;

   PyAf_Job_Type.tp_new    = PyType_GenericNew;
   PyAf_Block_Type.tp_new  = PyType_GenericNew;
   PyAf_Task_Type.tp_new   = PyType_GenericNew;
   PyAf_Cmd_Type.tp_new    = PyType_GenericNew;

   if( PyType_Ready( &PyAf_Job_Type    ) < 0 ) return RETVAR;
   if( PyType_Ready( &PyAf_Block_Type  ) < 0 ) return RETVAR;
   if( PyType_Ready( &PyAf_Task_Type   ) < 0 ) return RETVAR;
   if( PyType_Ready( &PyAf_Cmd_Type    ) < 0 ) return RETVAR;

#if PY_MAJOR_VERSION < 3
   module = Py_InitModule( "pyaf", pyaf_Methods);
#else
   module = PyModule_Create( &moduledef);
#endif
   if( module == NULL ) return RETVAR;

   Py_INCREF( &PyAf_Job_Type     );
   Py_INCREF( &PyAf_Block_Type   );
   Py_INCREF( &PyAf_Task_Type    );
   Py_INCREF( &PyAf_Cmd_Type     );

   PyModule_AddObject( module, "Job",     (PyObject*)&PyAf_Job_Type     );
   PyModule_AddObject( module, "Block",   (PyObject*)&PyAf_Block_Type   );
   PyModule_AddObject( module, "Task",    (PyObject*)&PyAf_Task_Type    );
   PyModule_AddObject( module, "Cmd",     (PyObject*)&PyAf_Cmd_Type     );


//############ Version: ########################
	#ifdef CGRU_REVISION
	#define STRINGIFY(x) #x
	#define EXPAND(x) STRINGIFY(x)
	PySys_WriteStdout("Afanasy build revision = \"%s\"\n", EXPAND(CGRU_REVISION));
	#endif
   PySys_WriteStdout("Python version = \"%d.%d.%d\"\n", PY_MAJOR_VERSION, PY_MINOR_VERSION, PY_MICRO_VERSION);
//###################################################

   return RETVAR;
}
