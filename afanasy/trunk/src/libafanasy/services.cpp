#include "services.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

char PYNAME_Module[]                = "afsrv";
char PYNAME_ModuleAll[]             = "__all__";

char PYNAME_ModuleServices[]        = "services";
char PYNAME_ListServices[]          = "service";
char PYNAME_FuncApplyCmdCapacity[]  = "applycmdcapacity";
char PYNAME_FuncApplyCmdHosts[]     = "applycmdhosts";
char PYNAME_FuncCheckFiles[]        = "checkfiles";

PyObject* Services::GetFunc( PyObject* module, char* name)
{
   PyObject *pFunc = PyObject_GetAttrString( module, name);
   if((pFunc == NULL) && (PyCallable_Check(pFunc)) == false)
   {
      if (PyErr_Occurred()) PyErr_Print();
      AFERRAR( "Cannot find function \"%s\"\n", name);
      if( pFunc) Py_DECREF( pFunc);
      return NULL;
   }
   return pFunc;
}

Services::Services( bool verbose):
   numservices(0),
   valid( false)
{
   //
   // loading module
   if( verbose) printf("\nImporting \"%s\" python module:\n", PYNAME_Module);
   PyObject *pName = PyString_FromString( PYNAME_Module);
   PyObj_Module = PyImport_Import( pName);
   Py_DECREF( pName);
   if( PyObj_Module == NULL)
   {
      PyErr_Print();
      AFERRAR( "Failed to load \"%s\"\n", PYNAME_Module);
      return;
   }
   //
   // getting services list
   PyObj_ListServices = PyObject_GetAttrString( PyObj_Module, PYNAME_ListServices);
   if( PyObj_ListServices == NULL)
   {
      if (PyErr_Occurred()) PyErr_Print();
      AFERRAR( "Cannot find list \"%s\"\n", PYNAME_ListServices);
      return;
   }
   if( PyList_Check( PyObj_ListServices) == false )
   {
      AFERRAR( "Object is not a list \"%s\"\n", PYNAME_ListServices);
      return;
   }
   // getting services count
   numservices = PyList_Size( PyObj_ListServices);
   if( numservices < 1)
   {
      AFERROR("Invalud number of services.\n");
      return;
   }
   if( verbose) printf("Services founded (%d):\n", numservices);
   PyObj_ModuleServices = PyObject_GetAttrString( PyObj_Module, PYNAME_ModuleServices);
   if( PyObj_ModuleServices == NULL)
   {
      PyErr_Print();
      AFERRAR( "Failed find services module\"%s.%s\"\n", PYNAME_Module, PYNAME_ModuleServices);
      return;
   }
   PyObj_ServicesAll = PyObject_GetAttrString( PyObj_ModuleServices, PYNAME_ModuleAll);
   if( PyObj_ServicesAll == NULL)
   {
      PyErr_Print();
      AFERRAR( "Failed find in services \"%s.%s.%s\"\n", PYNAME_Module, PYNAME_ModuleServices, PYNAME_ModuleAll);
      return;
   }
   if( PyList_Check( PyObj_ServicesAll) == false )
   {
      AFERRAR( "Object is not a list \"%s.%s.%s\"\n", PYNAME_Module, PYNAME_ModuleServices, PYNAME_ModuleAll);
      return;
   }
   if( numservices != PyList_Size( PyObj_ServicesAll))
   {
      AFERRAR( "Not all services from \"%s.%s.%s\" are allocated.\n", PYNAME_Module, PYNAME_ModuleServices, PYNAME_ModuleAll);
      return;
   }

   // allocating pointers
   PyObj_FuncApplyCmdCapacity = new PyObject*[numservices];
   PyObj_FuncApplyCmdHosts    = new PyObject*[numservices];
   PyObj_FuncCheckFiles       = new PyObject*[numservices];
   PyObj_ModuleService        = new PyObject*[numservices];
   PyObj_ClassService         = new PyObject*[numservices];
   for( int i = 0; i < numservices; ++i) PyObj_FuncApplyCmdCapacity[i] = NULL;
   for( int i = 0; i < numservices; ++i) PyObj_FuncApplyCmdHosts[i] = NULL;
   for( int i = 0; i < numservices; ++i) PyObj_FuncCheckFiles[i] = NULL;
   for( int i = 0; i < numservices; ++i) PyObj_ModuleService[i] = NULL;
   for( int i = 0; i < numservices; ++i) PyObj_ClassService[i] = NULL;

   // getting services attribures
   for( int i = 0; i < numservices; ++i)
   {
      // get service name
      PyObject * pName = PyList_GetItem( PyObj_ServicesAll, i);
      if( pName == NULL)
      {
         AFERRAR("Can't get service name at position %d\n", i)
         return;
      }
      char * servicename = PyString_AsString( pName);

      // get service reference
      PyObject * srv = PyList_GetItem( PyObj_ListServices, i);
      if( srv == NULL)
      {
         AFERRAR("Can't get item at position %d from \"%s\"", i, PYNAME_ListServices)
         return;
      }

      // get description
      PyObject * pDoc = PyObject_GetAttrString( srv, "__doc__");
      serviseDescriptions << PyString_AsString( pDoc);
      serviceNames << servicename;
      if( verbose) printf("%d. %s: %s\n",i , servicename, serviseDescriptions[i].toUtf8().data());

      // get service functions
      PyObj_FuncApplyCmdCapacity[i] = GetFunc( srv, PYNAME_FuncApplyCmdCapacity);
      PyObj_FuncApplyCmdHosts[i]    = GetFunc( srv, PYNAME_FuncApplyCmdHosts);
      PyObj_FuncCheckFiles[i]       = GetFunc( srv, PYNAME_FuncCheckFiles);

      //get service module
      PyObj_ModuleService[i] = PyObject_GetAttr( PyObj_ModuleServices, pName);
      if( PyObj_ModuleService[i] == NULL)
      {
         PyErr_Print();
         AFERRAR( "Failed find module \"%s\"\n", servicename);
         return;
      }

      //get service class
      PyObj_ClassService[i] = PyObject_GetAttr( PyObj_ModuleService[i], pName);
      if( PyObj_ClassService[i] == NULL)
      {
         AFERRAR( "Failed find class \"%s\"\n", servicename);
         return;
      }

      Py_DECREF( pName);
   }

   valid = true;
}

Services::~Services()
{
   for( int i = 0; i < numservices; i++)
   {
      if( PyObj_FuncApplyCmdCapacity[i]   != NULL) Py_XDECREF( PyObj_FuncApplyCmdCapacity[i]);
      if( PyObj_FuncApplyCmdHosts[i]      != NULL) Py_XDECREF( PyObj_FuncApplyCmdHosts[i]);
      if( PyObj_FuncCheckFiles[i]         != NULL) Py_XDECREF( PyObj_FuncCheckFiles[i]);
      if( PyObj_ModuleService[i]          != NULL) Py_XDECREF( PyObj_ModuleService[i]);
      if( PyObj_ClassService[i]           != NULL) Py_XDECREF( PyObj_ClassService[i]);
   }
   if( numservices > 0)
   {
      delete [] PyObj_FuncApplyCmdCapacity;
      delete [] PyObj_FuncApplyCmdHosts;
      delete [] PyObj_FuncCheckFiles;
      delete [] PyObj_ModuleService;
      delete [] PyObj_ClassService;
   }

   if( PyObj_ListServices) Py_XDECREF( PyObj_ListServices);

   Py_XDECREF( PyObj_ServicesAll);
   Py_XDECREF( PyObj_ModuleServices);
   Py_XDECREF( PyObj_Module);
}

PyObject * Services::getServiceClass( const QString & name) const
{
   for( int i = 0; i < numservices; i++) if( name == serviceNames[i]) return PyObj_ClassService[i];
   AFERRAR("Services::getServiceClass: No such class \"%s\"\n", name.toUtf8().data())
   return NULL;
}
