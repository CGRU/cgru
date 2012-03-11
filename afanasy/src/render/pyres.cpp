#include "pyres.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/afpynames.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/host.h"
/*
#if PY_MAJOR_VERSION < 3
#define PyBytes_Check PyString_Check
#define PyBytes_AsString PyString_AsString
#define PyBytes_FromString PyString_FromString
#define PyBytes_FromStringAndSize PyString_FromStringAndSize
#define PyLong_AsLong PyLong_AsLong
#define PyLong_FromLong PyInt_FromLong
#endif
*/
#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

PyRes::PyRes( const std::string & className, af::HostRes * hostRes):
   name( className),
   hres( hostRes),
   initialized( false)
{
   index = int( hres->custom.size());

   if( PyClass::init( AFPYNAMES::RES_CLASSESDIR, name.c_str(), NULL) == false) return;

   //Get functions:
   PyObj_FuncUpdate = getFunction( AFPYNAMES::RES_FUNC_UPDATE);
   if( PyObj_FuncUpdate == NULL ) return;

   hres->custom.push_back( new af::HostResMeter());
   initialized = true;
}

PyRes::~PyRes()
{
}

void PyRes::update()
{
   if( false == initialized ) return;

   PyObject * pTuple = PyObject_CallObject( PyObj_FuncUpdate, NULL);
   if( PyTuple_Check( pTuple))
   {
      if( PyTuple_Size( pTuple) == 16)
      {
         hres->custom[index]->value       = PyLong_AsLong(     PyTuple_GetItem( pTuple,  0));
         hres->custom[index]->valuemax    = PyLong_AsLong(     PyTuple_GetItem( pTuple,  1));
         hres->custom[index]->width       = PyLong_AsLong(     PyTuple_GetItem( pTuple,  2));
         hres->custom[index]->height      = PyLong_AsLong(     PyTuple_GetItem( pTuple,  3));
         hres->custom[index]->graphr      = PyLong_AsLong(     PyTuple_GetItem( pTuple,  4));
         hres->custom[index]->graphg      = PyLong_AsLong(     PyTuple_GetItem( pTuple,  5));
         hres->custom[index]->graphb      = PyLong_AsLong(     PyTuple_GetItem( pTuple,  6));
         hres->custom[index]->label       = PyBytes_AsString(  PyTuple_GetItem( pTuple,  7));
         hres->custom[index]->labelsize   = PyLong_AsLong(     PyTuple_GetItem( pTuple,  8));
         hres->custom[index]->labelr      = PyLong_AsLong(     PyTuple_GetItem( pTuple,  9));
         hres->custom[index]->labelg      = PyLong_AsLong(     PyTuple_GetItem( pTuple, 10));
         hres->custom[index]->labelb      = PyLong_AsLong(     PyTuple_GetItem( pTuple, 11));
         hres->custom[index]->bgcolorr    = PyLong_AsLong(     PyTuple_GetItem( pTuple, 12));
         hres->custom[index]->bgcolorg    = PyLong_AsLong(     PyTuple_GetItem( pTuple, 13));
         hres->custom[index]->bgcolorb    = PyLong_AsLong(     PyTuple_GetItem( pTuple, 14));
         hres->custom[index]->tooltip     = PyBytes_AsString(  PyTuple_GetItem( pTuple, 15));
      }
      else
         AFERRAR("PyRes::update: type=\"%s\" returned tuple size != 12\n", name.c_str());
   }
   else
      AFERRAR("PyRes::update: type=\"%s\" value is not a tuple\n", name.c_str());

   Py_DECREF( pTuple);
}
