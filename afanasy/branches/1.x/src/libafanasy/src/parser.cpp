#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <services.h>

using namespace af;

//#define AFOUTPUT
//#undef AFOUTPUT
#include <macrooutput.h>

extern char PYNAME_FuncPasre[];

Parser::Parser( const QString & type, int frames, bool verbose):
   initialized( false),
   numframes( frames),
   name( type),
   PyObj_ParserClass( NULL),
   PyObj_Instance( NULL),
   PyObj_FuncParse( NULL)
{
   PyObj_ParserClass = srv()->getParserClass( type);
   if( PyObj_ParserClass != NULL)
   {
      const char * name = type.toUtf8().data();
      PyObject *pArgs;
      pArgs = PyTuple_New( 1);
      PyTuple_SetItem( pArgs, 0, PyInt_FromLong( numframes));
      PyObj_Instance = PyInstance_New( PyObj_ParserClass, pArgs, NULL);
      Py_DECREF( pArgs);
      if( PyObj_Instance == NULL)
      {
         PyErr_Print();
         AFERRAR( "Failed to instance \"%s\"\n", name);
         return;
      }
      PyObj_FuncParse = PyObject_GetAttrString( PyObj_Instance, PYNAME_FuncPasre);
      if(( PyObj_FuncParse != NULL) && (PyCallable_Check( PyObj_FuncParse)))
      {
         initialized = true;
         if( verbose) printf("Parser class \"%s\" instanced. Parsing with \"%s.%s\" method.\n", name, name, PYNAME_FuncPasre);
      }
      else
      {
         if( PyErr_Occurred()) PyErr_Print();
         AFERRAR( "Cannot find function \"%s\" in \"%s\"\n", PYNAME_FuncPasre, name);
      }
   }
}

Parser::~Parser()
{
   if( PyObj_Instance) Py_XDECREF( PyObj_Instance);
}

bool Parser::parse(  char* data, int size,
                     int &percent, int &frame, int &percentframe,
                     bool &error, bool &warning) const
{
   bool result = false;
   if( data == NULL) return result;
   if( size < 1) return result;

   PyObject * pArgs = PyTuple_New( 1);
   PyTuple_SetItem( pArgs, 0, PyString_FromStringAndSize( data, size));
   PyObject * pTuple = PyObject_CallObject( PyObj_FuncParse, pArgs);
   if( PyTuple_Check( pTuple))
   {
      if( PyTuple_Size( pTuple) == 5)
      {
         percent        = PyInt_AsLong(    PyTuple_GetItem( pTuple, 0));
         frame          = PyInt_AsLong(    PyTuple_GetItem( pTuple, 1));
         percentframe   = PyInt_AsLong(    PyTuple_GetItem( pTuple, 2));
         error          = PyObject_IsTrue( PyTuple_GetItem( pTuple, 3));
         warning        = PyObject_IsTrue( PyTuple_GetItem( pTuple, 4));
         result         = true;
      }
      else
         AFERRAR("Parser::parse: type=\"%s\" returned tuple size != 5\n", name.toUtf8().data());
   }
   else
      AFERRAR("Parser::parse: type=\"%s\" value is not a tuple\n", name.toUtf8().data());

   Py_DECREF( pArgs);
   Py_DECREF( pTuple);
   return result;
}
