#include "pyaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void outError( const char * errMsg, const char * baseMsg = NULL)
{
   if( baseMsg )
      AFERRAR("%s: %s", baseMsg, errMsg)
   else
      AFERRAR("%s", errMsg)
}

bool PyAf::GetString( PyObject * obj, std::string & str, const char * errMsg)
{
   if( PyBytes_Check(obj))
   {
      str = PyBytes_AsString( obj);
      return true;
   }
#if PY_MAJOR_VERSION >= 3
   else if( PyUnicode_Check(obj))
   {
      obj = PyUnicode_AsUTF8String( obj);
      if( obj == NULL )
      {
         outError("String object unt8 encoding problems.", errMsg);
         return false;
      }
      str = PyBytes_AsString( obj);
      Py_DECREF( obj);
      return true;
   }
#endif
   outError("Object is not a bytes array or string.", errMsg);

   return false;
}
