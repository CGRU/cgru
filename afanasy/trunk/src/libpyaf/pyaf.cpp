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

bool PyAf::GetInteger( PyObject * obj, long long & value, const char * errMsg)
{
   if( PyLong_Check( obj))
   {
      value = PyLong_AsLongLong( obj);
      return true;
   }
#if PY_MAJOR_VERSION < 3
   if( PyInt_Check( obj))
   {
      value = PyInt_AsLong( obj);
      return true;
   }
#endif
   outError("Object is not a number.", errMsg);
   return false;
}

bool PyAf::GetString( PyObject * obj, std::string & str, const char * errMsg)
{
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
         outError("PyUnicode object encoding problems.", errMsg);
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
   outError("Object is not a bytes array, string or unicode.", errMsg);

   return false;
}

bool PyAf::GetStrings( PyObject * obj, std::vector<std::string> & list, int min, int max, const char * errMsg)
{
   if( false == PyTuple_Check( obj))
   {
      outError("Object is not a tuple.", errMsg);
      return false;
   }
   int size = PyTuple_GET_SIZE( obj);
   for( int i = 0; i < size; i++)
   {
      PyObject * obj_str = PyTuple_GetItem( obj, i);
      if( obj_str == NULL)
      {
         outError("Invalid object in a tuple.", errMsg);
         return false;
      }
      std::string str;
      if( false == PyAf::GetString( obj_str, str, errMsg))
      {
         outError("Invalid object in a tuple.", errMsg);
         return false;
      }
      list.push_back( str);
   }
   if(( min > -1 ) && ( list.size() < min ))
   {
      outError("Tuple size less than a minimum.", errMsg);
      return false;
   }
   if(( max > -1 ) && ( list.size() > max ))
   {
      outError("Tuple size greater than a maximum.", errMsg);
      return false;
   }
   return true;
}
