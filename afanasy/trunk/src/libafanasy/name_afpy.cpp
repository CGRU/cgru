#include "name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

bool af::PyGetString( PyObject * obj, std::string & str, const char * errMsg)
{
   if( obj == NULL)
   {
      outError("af::PyGetString: Object is NULL.", errMsg);
      return false;
   }
   if( obj == Py_None)
   {
      outError("af::PyGetString: Object is None.", errMsg);
      return false;
   }
#if PY_MAJOR_VERSION < 3
   if( PyString_Check(obj))
   {
      str = PyString_AsString( obj);
      return true;
   }
#else
   if( PyBytes_Check(obj))
   {
      str = PyBytes_AsString( obj);
      return true;
   }
#endif
   if( PyUnicode_Check(obj))
   {
      obj = PyUnicode_AsUTF8String( obj);
      if( obj == NULL )
      {
         outError("af::PyGetString: PyUnicode object encoding problems.", errMsg);
         return false;
      }
#if PY_MAJOR_VERSION < 3
      str = PyString_AsString( obj);
#else
      str = PyBytes_AsString( obj);
#endif
      Py_DECREF( obj);
      return true;
   }
   outError("af::PyGetString: Object is not a bytes array, string or unicode.", errMsg);

   return false;
}
