#pragma once

#include "../libafanasy/pyclass.h"

class PyRes: public af::PyClass
{
public:
	PyRes(const std::string & i_className, af::HostRes * i_hostRes);
	~PyRes();

	inline bool isInitialized() const {return m_initialized;}

	void update();

private:
	std::string m_name;
	af::HostRes * m_hres;
	int m_index;

	PyObject * m_PyObj_FuncUpdate;

	bool m_initialized;
};
