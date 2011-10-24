#pragma once

#include "../libafanasy/pyclass.h"

class PyRes: public af::PyClass
{
public:
   PyRes( const std::string & className, af::HostRes * hostRes);
   ~PyRes();

   inline bool isInitialized() const { return initialized;}

   void update();

private:
   std::string name;
   af::HostRes * hres;
   int index;

   PyObject* PyObj_FuncUpdate;

   bool initialized;
};
