#pragma once

#include "name_af.h"

#include "pymodule.h"

namespace af
{
class PyClass: public PyModule
{
public:
	/// Constructor do nothing
	PyClass();

	/// Deincrement all objects references ( and functions too )
	~PyClass();

protected:
	/// Import and reload module, find and instance class with provided arguments (arguments will be destoyed if not NULL)
	bool init(const std::string & i_dir, const std::string & i_name, PyObject * i_args);

	/// Get function (get attribute by name and check if it callable)
	/** All functions pointers are stored in this class and will be deleted in destructor
	**/
	PyObject * getFunction(const std::string & i_name);

private:
//	std::string modulename;           ///< Store "module.class" string to output with errors for indentitication
//	PyObject * PyObj_Module;      ///< Module object
	PyObject * PyObj_Type;        ///< Class type object
	PyObject * PyObj_Instance;    ///< Class instance object

	/// All functions list (stored on "getFunction"), will be deleted in destructor
//	std::list<PyObject*> PyObj_FuncList;
};
}
