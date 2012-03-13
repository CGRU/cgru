#pragma once

#include <Python.h>

#include "../libafanasy/pyclass.h"

class JobAf;

/// Job Python API
class JobPy: public af::PyClass
{
public:

/// Construct job
   JobPy( JobAf * jobPtr);
   ~JobPy();

/// Whether job python class was initialized successfully.
   inline bool isInitialized() const { return initialized;}

private:
   bool initialized;             ///< Whether job python class was initialized successfully.

   JobAf * job;

   PyObject * PyObj_FuncConstuct;
   PyObject * PyObj_FuncAddBlock;
   PyObject * PyObj_FuncDestroy;
};
