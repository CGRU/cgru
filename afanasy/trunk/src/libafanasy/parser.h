#pragma once

#include <Python.h>

#include <QtCore/QString>

#include "../include/aftypes.h"

#include "name_af.h"
#include "pyclass.h"

namespace af
{
class Parser: public PyClass
{
public:
   Parser( const QString & type, int frames = 1);
   ~Parser();

   inline bool isInitialized() const { return initialized;}

   bool parse( char* data, int size,
               int &percent, int &frame, int &percentframe,
               bool &error, bool &warning) const;

private:
   bool initialized;
   int  numframes;
   QString name;

   PyObject* PyObj_FuncParse;
};
}
