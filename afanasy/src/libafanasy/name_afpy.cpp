#include "name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

bool af::PyGetString( PyObject * i_obj, std::string & o_str, const char * i_err_info)
{
	if( i_obj == NULL)
	{
		outError("af::PyGetString: Object is NULL.", i_err_info);
		return false;
	}
	if( i_obj == Py_None)
	{
		outError("af::PyGetString: Object is None.", i_err_info);
		return false;
	}
#if PY_MAJOR_VERSION < 3
	if( PyString_Check(i_obj))
	{
		o_str = PyString_AsString( i_obj);
		return true;
	}
#else
	if( PyBytes_Check(i_obj))
	{
		o_str = PyBytes_AsString( i_obj);
		return true;
	}
#endif
	if( PyUnicode_Check(i_obj))
	{
		i_obj = PyUnicode_AsUTF8String( i_obj);
		if( i_obj == NULL )
		{
			outError("af::PyGetString: PyUnicode object encoding problems.", i_err_info);
			return false;
		}
#if PY_MAJOR_VERSION < 3
		o_str = PyString_AsString( i_obj);
#else
		o_str = PyBytes_AsString( i_obj);
#endif
		Py_DECREF( i_obj);
		return true;
	}
	outError("af::PyGetString: Object is not a bytes array, string or unicode.", i_err_info);

	return false;
}

bool af::PyGetStringList( PyObject * i_obj, std::vector<std::string> & o_list, const char * i_err_info)
{
	o_list.clear();

	if( PyList_Check( i_obj) != true )
	{
		outError("af::PyGetStringList: Object is not a list.");
		return false;
	}

	int size = PyList_Size( i_obj);
	for( int i = 0; i < size; i++)
	{
		std::string str;
		if( af::PyGetString( PyList_GetItem( i_obj, i), str))
			o_list.push_back( str);
	}

	return true;
}

