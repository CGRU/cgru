#pragma once

#include <Python.h>

#include <QtCore/QStringList>


namespace af
{
class Services
{
public:
   Services( bool verbose = false);
   ~Services();

   inline bool isValid()  const { return valid; }

   PyObject * getServiceClass( const QString & name) const;

private:
   PyObject* PyObj_Module;

   int numservices;
   QStringList serviceNames;
   QStringList serviseDescriptions;
   PyObject  * PyObj_ModuleServices;
   PyObject  * PyObj_ServicesAll;
   PyObject  * PyObj_ListServices;
   PyObject ** PyObj_ModuleService;
   PyObject ** PyObj_ClassService;
   PyObject ** PyObj_FuncApplyCmdCapacity;
   PyObject ** PyObj_FuncApplyCmdHosts;
   PyObject ** PyObj_FuncCheckFiles;

   bool valid;

private:
   static PyObject* GetFunc(  PyObject* module, char* name);
};
}
