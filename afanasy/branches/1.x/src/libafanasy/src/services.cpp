#include "services.h"

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

using namespace af;

char PYNAME_Module[]                = "afsrv";
char PYNAME_ModuleAll[]             = "__all__";

char PYNAME_ModuleServices[]        = "services";
char PYNAME_ListServices[]          = "service";
char PYNAME_FuncApplyCmdCapacity[]  = "applycmdcapacity";
char PYNAME_FuncApplyCmdHosts[]     = "applycmdhosts";
char PYNAME_FuncCheckFiles[]        = "checkfiles";

char PYNAME_ModuleParsers[]         = "parsers";
char PYNAME_ListParsers[]           = "parser";
char PYNAME_FuncPasre[]             = "parse";

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
   numparsers(0),
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

   //
   // getting parsers list
   PyObj_ListParsers = PyObject_GetAttrString( PyObj_Module, PYNAME_ListParsers);
   if( PyObj_ListParsers == NULL)
   {
      if (PyErr_Occurred()) PyErr_Print();
      AFERRAR( "Cannot find list \"%s\"\n", PYNAME_ListParsers);
      return;
   }
   if( PyList_Check( PyObj_ListParsers) == false )
   {
      AFERRAR( "Object is not a list \"%s\"\n", PYNAME_ListParsers);
      return;
   }
   // getting parsers count
   numparsers = PyList_Size( PyObj_ListParsers);
   if( numparsers < 1)
   {
      AFERROR("Invalud number of parsers.\n");
      return;
   }
   if( verbose) printf("Parsers founded (%d):\n", numparsers);
   PyObj_ModuleParsers = PyObject_GetAttrString( PyObj_Module, PYNAME_ModuleParsers);
   if( PyObj_ModuleParsers == NULL)
   {
      PyErr_Print();
      AFERRAR( "Failed find parsers module\"%s.%s\"\n", PYNAME_Module, PYNAME_ModuleParsers);
      return;
   }
   PyObj_ParsersAll = PyObject_GetAttrString( PyObj_ModuleParsers, PYNAME_ModuleAll);
   if( PyObj_ParsersAll == NULL)
   {
      PyErr_Print();
      AFERRAR( "Failed find in parsers \"%s.%s.%s\"\n", PYNAME_Module, PYNAME_ModuleParsers, PYNAME_ModuleAll);
      return;
   }
   if( PyList_Check( PyObj_ParsersAll) == false )
   {
      AFERRAR( "Object is not a list \"%s.%s.%s\"\n", PYNAME_Module, PYNAME_ModuleParsers, PYNAME_ModuleAll);
      return;
   }
   if( numparsers != PyList_Size( PyObj_ParsersAll))
   {
      AFERRAR( "Not all services from \"%s.%s.%s\" are allocated.\n", PYNAME_Module, PYNAME_ModuleParsers, PYNAME_ModuleAll);
      return;
   }

   // allocating pointers
   PyObj_FuncParse    = new PyObject*[numparsers];
   PyObj_ModuleParser = new PyObject*[numparsers];
   PyObj_ClassParser  = new PyObject*[numparsers];
   for( int i = 0; i < numparsers; ++i) PyObj_FuncParse[i] = NULL;
   for( int i = 0; i < numparsers; ++i) PyObj_ModuleParser[i] = NULL;
   for( int i = 0; i < numparsers; ++i) PyObj_ClassParser[i] = NULL;

   // getting parser attribures
   for( int i = 0; i < numparsers; ++i)
   {
      // get parser name
      PyObject * pName = PyList_GetItem( PyObj_ParsersAll, i);
      if( pName == NULL)
      {
         AFERRAR("Can't get parser name at position %d\n", i)
         return;
      }
      char * parsername = PyString_AsString( pName);

      // get parser reference
      PyObject * prs = PyList_GetItem( PyObj_ListParsers, i);
      if( prs == NULL)
      {
         AFERRAR("Can't get item at position %d from \"%s\"", i, PYNAME_ListParsers)
         return;
      }

      // get description
      PyObject * pDoc = PyObject_GetAttrString( prs, "__doc__");
      parserDescriptions << PyString_AsString( pDoc);
      parserNames << parsername;
      if( verbose) printf("%d. %s: %s\n", i, parsername, parserDescriptions[i].toUtf8().data());

      // get parse function
      PyObj_FuncParse[i] = GetFunc( prs, PYNAME_FuncPasre);

      //get parser module
      PyObj_ModuleParser[i] = PyObject_GetAttr( PyObj_ModuleParsers, pName);
      if( PyObj_ModuleParser[i] == NULL)
      {
         PyErr_Print();
         AFERRAR( "Failed find module \"%s\"\n", parsername);
         return;
      }

      //get parser class
      PyObj_ClassParser[i] = PyObject_GetAttr( PyObj_ModuleParser[i], pName);
      if( PyObj_ClassParser[i] == NULL)
      {
         AFERRAR( "Failed find class \"%s\"\n", parsername);
         return;
      }
/*
PyObject * pArgs = PyTuple_New( 1);
PyTuple_SetItem( pArgs, 0, PyInt_FromLong( 1));
PyObject * PyObj_Instance = PyInstance_New( PyObj_ClassParser[i], pArgs, NULL);
Py_DECREF( pArgs);
if( PyObj_Instance == NULL)
{
   PyErr_Print();
   AFERRAR( "Failed to instance \"%s\"\n", parsername);
   return;
}
*/
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

   for( int i = 0; i < numparsers;  i++)
   {
      if( PyObj_FuncParse[i]    != NULL) Py_XDECREF( PyObj_FuncParse[i]);
      if( PyObj_ModuleParser[i] != NULL) Py_XDECREF( PyObj_ModuleParser[i]);
      if( PyObj_ClassParser[i]  != NULL) Py_XDECREF( PyObj_ClassParser[i]);
   }
   if( numparsers  > 0)
   {
      delete [] PyObj_FuncParse;
      delete [] PyObj_ModuleParser;
      delete [] PyObj_ClassParser;
   }

   if( PyObj_ListServices) Py_XDECREF( PyObj_ListServices);
   if( PyObj_ListParsers)  Py_XDECREF( PyObj_ListParsers );

   Py_XDECREF( PyObj_ServicesAll);
   Py_XDECREF( PyObj_ModuleParsers);
   Py_XDECREF( PyObj_ModuleServices);
   Py_XDECREF( PyObj_Module);
}

PyObject * Services::getParserClass( const QString & name) const
{
   for( int i = 0; i < numparsers; i++) if( name == parserNames[i]) return PyObj_ClassParser[i];
   AFERRAR("Services::getParserClass: No such class \"%s\"\n", name.toUtf8().data())
   return NULL;
}

PyObject * Services::getServiceClass( const QString & name) const
{
   for( int i = 0; i < numservices; i++) if( name == serviceNames[i]) return PyObj_ClassService[i];
   AFERRAR("Services::getServiceClass: No such class \"%s\"\n", name.toUtf8().data())
   return NULL;
}
