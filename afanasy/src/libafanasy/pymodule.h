#pragma once

#include "name_af.h"

namespace af
{
class PyModule
{
public:
	/// Constructor do nothing
	PyModule();

	/// Deincrement all objects references ( and functions too )
	~PyModule();

//protected:
	/// Import and reload module, find and instance class with provided arguments (arguments will be destoyed if not NULL)
	bool init(const std::string & i_dir, const std::string & i_name);

	/// Get function (get attribute by name and check if it callable)
	/// All functions pointers are stored in this class and will be deleted in destructor
	PyObject * getFunction(const std::string & i_name);

	inline std::string & getModuleName() {return m_modulename;}
	inline PyObject * getModuleObj() {return m_PyObj_Module;}

protected:
	/// All functions list (stored on "getFunction"), will be deleted in destructor
	std::list<PyObject*> m_PyObj_FuncList;

private:
	std::string m_modulename;     ///< Store "module.class" string to output with errors for indentitication
	PyObject *  m_PyObj_Module;   ///< Module object
};
}
