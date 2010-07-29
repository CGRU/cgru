#pragma once

#include <Python.h>

#include <QtCore/QString>

#include "../libafanasy/pyclass.h"

class PyRes: public af::PyClass
{
public:
   PyRes( const QString & className, af::HostRes * hostRes);
   ~PyRes();

   inline bool isInitialized() const { return initialized;}

   void update();

private:
   QString name;
   af::HostRes * hres;
   int index;

   PyObject* PyObj_FuncUpdate;

   bool initialized;
};
