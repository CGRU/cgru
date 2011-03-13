#include <Python.h>
#include <structmember.h>

#include "pyafjob.h"
#include "pyafblock.h"
#include "pyaftask.h"
#include "pyafcmd.h"

static PyMethodDef PyAf_Methods[] = {
   { 0, 0, 0, 0 }// Sentinel
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initpyaf(void)
{
   PyObject * module;

   PyAf_Job_Type.tp_new    = PyType_GenericNew;
   PyAf_Block_Type.tp_new  = PyType_GenericNew;
   PyAf_Task_Type.tp_new   = PyType_GenericNew;
   PyAf_Cmd_Type.tp_new    = PyType_GenericNew;

   if( PyType_Ready( &PyAf_Job_Type    ) < 0 ) return;
   if( PyType_Ready( &PyAf_Block_Type  ) < 0 ) return;
   if( PyType_Ready( &PyAf_Task_Type   ) < 0 ) return;
   if( PyType_Ready( &PyAf_Cmd_Type    ) < 0 ) return;

   module = Py_InitModule( "pyaf", PyAf_Methods);

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
   printf("Afanasy build revision = \"%d\"\n", CGRU_REVISION);
#endif
   printf("Python version = \"%d.%d.%d\"\n", PY_MAJOR_VERSION, PY_MINOR_VERSION, PY_MICRO_VERSION);
//###################################################
}
