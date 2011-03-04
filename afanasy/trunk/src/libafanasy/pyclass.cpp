#include "pyclass.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

PyClass::PyClass():
   PyObj_Module( NULL),
   PyObj_Type( NULL),
   PyObj_Instance( NULL)
{
}

bool PyClass::init( const std::string & dir, const std::string & name, PyObject * initArgs)
{
   modulename = dir + "." + name;
   AFINFA("Instancing pyclass '%s'\n", modulename.c_str());

   //
   // Load module
   PyObject * _PyObj_Module_ = PyImport_ImportModule( modulename.c_str());
   if(_PyObj_Module_== NULL)
   {
      if( PyErr_Occurred()) PyErr_Print();
      AFERRAR("Failed to import module '%s'\n", modulename.c_str());
      return false;
   }
   // Reload module
   PyObj_Module = PyImport_ReloadModule(_PyObj_Module_);
   if( PyObj_Module == NULL)
   {
      if( PyErr_Occurred()) PyErr_Print();
      AFERRAR("Failed to reload module '%s'\n", modulename.c_str());
      return false;
   }
   Py_DECREF( _PyObj_Module_);

   //Get class type
   PyObj_Type = PyObject_GetAttrString( PyObj_Module, name.c_str());
   if( PyObj_Type == NULL)
   {
      if( PyErr_Occurred()) PyErr_Print();
      AFERRAR("Failed find class '%s'\n", name.c_str());
      return false;
   }

   // Get class instance
   PyObj_Instance = PyInstance_New( PyObj_Type, initArgs, NULL);
   if( PyObj_Instance == NULL)
   {
      if( PyErr_Occurred()) PyErr_Print();
      AFERRAR("Failed to instance '%s'\n", name.c_str());
      return false;
   }
   if( initArgs) Py_DECREF( initArgs);

   return true;
}

PyClass::~PyClass()
{
   // Decrement functions references
   for( std::list<PyObject*>::iterator it = PyObj_FuncList.begin(); it != PyObj_FuncList.end(); it++)
      Py_XDECREF( *it);

   if( PyObj_Instance   ) Py_XDECREF( PyObj_Instance  );
   if( PyObj_Type       ) Py_XDECREF( PyObj_Type      );
   if( PyObj_Module     ) Py_XDECREF( PyObj_Module    );
}

PyObject * PyClass::getFunction( const std::string & name)
{
   // Get attribute object:
   PyObject * PyObj_Func = PyObject_GetAttrString( PyObj_Instance, name.c_str());
   if( PyObj_Func == NULL )
   {
      if( PyErr_Occurred()) PyErr_Print();
      AFERRAR("Cannot find function '%s' in '%s'\n", name.c_str(), modulename.c_str());
      return NULL;
   }

   // Check for callable:
   if( PyCallable_Check( PyObj_Func) == false)
   {
      Py_XDECREF( PyObj_Func);
      AFERRAR("Attribute '%s' in object '%s' is not callable\n", name.c_str(), modulename.c_str());
      return NULL;
   }

   // Store pointer to decrement reference on destuction
   PyObj_FuncList.push_back( PyObj_Func);

   return PyObj_Func;
}
