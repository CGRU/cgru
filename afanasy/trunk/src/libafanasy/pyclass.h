#pragma once

//#include <Python.h>
#include "name_af.h"

namespace af
{
class PyClass
{
public:
   /// Constructor do nothing
   PyClass();

   /// Deincrement all objects references ( and functions too )
   ~PyClass();

protected:
   /// Import and reload module, find and instance class with provided arguments (arguments will be destoyed if not NULL)
   bool init( const std::string & dir, const std::string & name, PyObject * initArgs);

   /// Get function (get attribute by name and check if it callable)
   /** All functions pointers are stored in this class and will be deleted in destructor
   **/
   PyObject * getFunction( const std::string & name);

private:
   std::string modulename;           ///< Store "module.class" string to output with errors for indentitication
   PyObject * PyObj_Module;      ///< Module object
   PyObject * PyObj_Type;        ///< Class type object
   PyObject * PyObj_Instance;    ///< Class instance object

   /// All functions list (stored on "getFunction"), will be deleted in destructor
   std::list<PyObject*> PyObj_FuncList;
};
}
