#include "pyres.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/afpynames.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/host.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

PyRes::PyRes( const std::string & className, af::HostRes * hostRes):
   name( className),
   hres( hostRes),
   initialized( false)
{
   index = hres->custom.size();

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
         hres->custom[index]->value       = PyInt_AsLong(      PyTuple_GetItem( pTuple,  0));
         hres->custom[index]->valuemax    = PyInt_AsLong(      PyTuple_GetItem( pTuple,  1));
         hres->custom[index]->width       = PyInt_AsLong(      PyTuple_GetItem( pTuple,  2));
         hres->custom[index]->height      = PyInt_AsLong(      PyTuple_GetItem( pTuple,  3));
         hres->custom[index]->graphr      = PyInt_AsLong(      PyTuple_GetItem( pTuple,  4));
         hres->custom[index]->graphg      = PyInt_AsLong(      PyTuple_GetItem( pTuple,  5));
         hres->custom[index]->graphb      = PyInt_AsLong(      PyTuple_GetItem( pTuple,  6));
         hres->custom[index]->label       = PyString_AsString( PyTuple_GetItem( pTuple,  7));
         hres->custom[index]->labelsize   = PyInt_AsLong(      PyTuple_GetItem( pTuple,  8));
         hres->custom[index]->labelr      = PyInt_AsLong(      PyTuple_GetItem( pTuple,  9));
         hres->custom[index]->labelg      = PyInt_AsLong(      PyTuple_GetItem( pTuple, 10));
         hres->custom[index]->labelb      = PyInt_AsLong(      PyTuple_GetItem( pTuple, 11));
         hres->custom[index]->bgcolorr    = PyInt_AsLong(      PyTuple_GetItem( pTuple, 12));
         hres->custom[index]->bgcolorg    = PyInt_AsLong(      PyTuple_GetItem( pTuple, 13));
         hres->custom[index]->bgcolorb    = PyInt_AsLong(      PyTuple_GetItem( pTuple, 14));
         hres->custom[index]->tooltip     = PyString_AsString( PyTuple_GetItem( pTuple, 15));
      }
      else
         AFERRAR("PyRes::update: type=\"%s\" returned tuple size != 12\n", name.c_str());
   }
   else
      AFERRAR("PyRes::update: type=\"%s\" value is not a tuple\n", name.c_str());

   Py_DECREF( pTuple);
}
