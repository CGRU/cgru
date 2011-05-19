#include "parser.h"

#include "../include/afpynames.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Parser::Parser( const std::string & type, int frames):
   initialized( false),
   numframes( frames),
   name( type),
   PyObj_FuncParse( NULL)
{
   PyObject * args = PyTuple_New( 1);
   PyTuple_SetItem( args, 0, PyLong_FromLong( numframes));

   if( PyClass::init( AFPYNAMES::PARSER_CLASSESDIR, name, args) == false) return;

   //Get function
   PyObj_FuncParse = getFunction( AFPYNAMES::PARSER_FUNC_PARSE);
   if( PyObj_FuncParse == NULL ) return;

   initialized = true;
}

Parser::~Parser()
{
}

bool Parser::parse(  std::string & data,
                     int & percent, int & frame, int & percentframe,
                     bool & warning, bool & error, bool & badresult) const
{
   bool result = false;
   if( data.size() < 1) return result;

   PyObject * pArgs = PyTuple_New( 1);
   PyTuple_SetItem( pArgs, 0, PyBytes_FromStringAndSize( data.data(), data.size()));
   PyObject * pTuple = PyObject_CallObject( PyObj_FuncParse, pArgs);
   if( PyTuple_Check( pTuple))
   {
      if( PyTuple_Size( pTuple) == 7)
      {
         percent        = PyLong_AsLong(   PyTuple_GetItem( pTuple, 1));
         frame          = PyLong_AsLong(   PyTuple_GetItem( pTuple, 2));
         percentframe   = PyLong_AsLong(   PyTuple_GetItem( pTuple, 3));
         warning        = PyObject_IsTrue( PyTuple_GetItem( pTuple, 4));
         error          = PyObject_IsTrue( PyTuple_GetItem( pTuple, 5));
         badresult      = PyObject_IsTrue( PyTuple_GetItem( pTuple, 6));
         PyObject * output = PyTuple_GetItem( pTuple, 0);
         if( PyBytes_Check( output)) data = PyBytes_AsString( output);
         result = true;
      }
      else
         AFERRAR("Parser::parse: type=\"%s\" returned tuple size != 5\n", name.c_str());
   }
   else
      AFERRAR("Parser::parse: type=\"%s\" value is not a tuple\n", name.c_str());

   Py_DECREF( pArgs);
   Py_DECREF( pTuple);
   return result;
}
