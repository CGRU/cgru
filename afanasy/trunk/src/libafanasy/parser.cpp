#include "parser.h"

#include "../include/afpynames.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Parser::Parser( const QString & type, int frames):
   initialized( false),
   numframes( frames),
   name( type),
   PyObj_FuncParse( NULL)
{
   PyObject * args = PyTuple_New( 1);
   PyTuple_SetItem( args, 0, PyInt_FromLong( numframes));

   if( PyClass::init( AFPYNAMES::PARSER_CLASSESDIR, name.toUtf8().data(), args) == false) return;

   //Get function
   PyObj_FuncParse = getFunction( AFPYNAMES::PARSER_FUNC_PARSE);
   if( PyObj_FuncParse == NULL ) return;

   initialized = true;
}

Parser::~Parser()
{
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
