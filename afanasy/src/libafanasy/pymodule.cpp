#include "pymodule.h"

#include "environment.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

PyModule::PyModule():
	m_PyObj_Module(NULL)
{
}

bool PyModule::init(const std::string & i_dir, const std::string & i_name)
{
	m_modulename = i_dir + "." + i_name;

	// Load module
	PyObject * _PyObj_Module_ = PyImport_ImportModule(m_modulename.c_str());
	if (_PyObj_Module_== NULL)
	{
		if (PyErr_Occurred()) PyErr_Print();
			return false;
	}

	// Reload module
	m_PyObj_Module = PyImport_ReloadModule(_PyObj_Module_);
	if (m_PyObj_Module == NULL)
	{
		if (PyErr_Occurred()) PyErr_Print();
			return false;
	}
	Py_DECREF(_PyObj_Module_);

	 if (af::Environment::isVerboseMode())
			std::cout << "Module \"" << m_modulename << "\" imported." << std::endl;

	return true;
}

PyModule::~PyModule()
{
	// Decrement functions references
	for (std::list<PyObject*>::iterator it = m_PyObj_FuncList.begin(); it != m_PyObj_FuncList.end(); it++)
		Py_XDECREF(*it);

	if (m_PyObj_Module) Py_XDECREF(m_PyObj_Module);
}

PyObject * PyModule::getFunction(const std::string & i_name)
{
	// Get attribute object:
	PyObject * PyObj_Func = PyObject_GetAttrString(m_PyObj_Module, i_name.c_str());
	if (PyObj_Func == NULL)
	{
		if (PyErr_Occurred()) PyErr_Print();
		AF_ERR << "Cannot find function '" << i_name << "' in '" << m_modulename << "'";
		return NULL;
	}

	// Check for callable:
	if (PyCallable_Check(PyObj_Func) == false)
	{
		Py_XDECREF(PyObj_Func);
		AF_ERR << "Attribute '" << i_name << "' in object '" << m_modulename << "' is not callable.";
		return NULL;
	}

	// Store pointer to decrement reference on destuction
	m_PyObj_FuncList.push_back(PyObj_Func);

	return PyObj_Func;
}
