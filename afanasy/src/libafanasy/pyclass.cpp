#include "pyclass.h"

#include "environment.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

PyClass::PyClass():
	PyObj_Type(NULL),
	PyObj_Instance(NULL)
{
}

bool PyClass::init(const std::string & i_dir, const std::string & i_name, PyObject * i_args)
{
	// Init module
	if (false == PyModule::init(i_dir, i_name))
		return false;

	//Get class type
	PyObj_Type = PyObject_GetAttrString(getModuleObj(), i_name.c_str());
	if (PyObj_Type == NULL)
	{
		if (PyErr_Occurred()) PyErr_Print();
		return false;
	}

	// Get class instance
	PyObj_Instance = PyObject_CallObject(PyObj_Type, i_args);
	if (PyObj_Instance == NULL)
	{
		if (PyErr_Occurred()) PyErr_Print();
		return false;
	}
	if (i_args) Py_DECREF(i_args);

	return true;
}

PyClass::~PyClass()
{
	if (PyObj_Instance) Py_XDECREF(PyObj_Instance);
	if (PyObj_Type    ) Py_XDECREF(PyObj_Type    );
}

PyObject * PyClass::getFunction(const std::string & i_name)
{
	// Get attribute object:
	PyObject * PyObj_Func = PyObject_GetAttrString(PyObj_Instance, i_name.c_str());
	if (PyObj_Func == NULL)
	{
		if (PyErr_Occurred()) PyErr_Print();
		AF_ERR << "Cannot find function '" << i_name << "' in '" << getModuleName() << "'";
		return NULL;
	}

	// Check for callable:
	if (PyCallable_Check(PyObj_Func) == false)
	{
		Py_XDECREF(PyObj_Func);
		AF_ERR << "Attribute '" << i_name << "' in object '" << getModuleName() << "' is not callable.";
		return NULL;
	}

	// Store pointer to decrement reference on destuction
	m_PyObj_FuncList.push_back(PyObj_Func);

	return PyObj_Func;
}
