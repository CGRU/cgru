#include "pyres.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/afpynames.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/host.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

PyRes::PyRes(const std::string & i_className, af::HostRes * i_hostRes):
	m_name(i_className),
	m_hres(i_hostRes),
	m_initialized(false)
{
	m_index = int(m_hres->custom.size());

	if (PyClass::init(AFPYNAMES::RES_CLASSESDIR, m_name.c_str(), NULL) == false)
		return;

	//Get functions:
	m_PyObj_FuncUpdate = getFunction(AFPYNAMES::RES_FUNC_UPDATE);
	if (m_PyObj_FuncUpdate == NULL)
		return;

	m_hres->custom.push_back(new af::HostResMeter());
	m_initialized = true;
}

PyRes::~PyRes()
{
}

void PyRes::update()
{
	if (false == m_initialized)
		return;

	PyObject * pClass = PyObject_CallObject(m_PyObj_FuncUpdate, NULL);

	if (pClass == NULL)
	{
		AF_ERR << "PyRes::update['" << m_name << "']: returned NULL";
		return;
	}

	if (pClass == Py_None)
	{
		AF_ERR << "PyRes::update['" << m_name << "']: returned None";
		return;
	}

	std::string err = std::string("PyRes::update['" + m_name + "']: ");


	int value, valuemax, width, height, graphr, graphg, graphb, labelsize, labelr, labelg, labelb, bgcolorr, bgcolorg, bgcolorb = 0;
	std::string label, tooltip;

	if (false == af::PyGetAttrInt(pClass,"value",     value,     err)) return;
	if (false == af::PyGetAttrInt(pClass,"valuemax",  valuemax,  err)) return;
	if (false == af::PyGetAttrInt(pClass,"width",     width,     err)) return;
	if (false == af::PyGetAttrInt(pClass,"height",    height,    err)) return;
	if (false == af::PyGetAttrInt(pClass,"graphr",    graphr,    err)) return;
	if (false == af::PyGetAttrInt(pClass,"graphg",    graphg,    err)) return;
	if (false == af::PyGetAttrStr(pClass,"label",     label,     err)) return;
	if (false == af::PyGetAttrInt(pClass,"labelsize", labelsize, err)) return;
	if (false == af::PyGetAttrInt(pClass,"labelr",    labelr,    err)) return;
	if (false == af::PyGetAttrInt(pClass,"labelg",    labelg,    err)) return;
	if (false == af::PyGetAttrInt(pClass,"labelb",    labelb,    err)) return;
	if (false == af::PyGetAttrInt(pClass,"bgcolorr",  bgcolorr,  err)) return;
	if (false == af::PyGetAttrInt(pClass,"bgcolorg",  bgcolorg,  err)) return;
	if (false == af::PyGetAttrInt(pClass,"bgcolorb",  bgcolorb,  err)) return;
	if (false == af::PyGetAttrStr(pClass,"tooltip",   tooltip,   err)) return;

	m_hres->custom[m_index]->value     = value;
	m_hres->custom[m_index]->valuemax  = valuemax;
	m_hres->custom[m_index]->width     = width;
	m_hres->custom[m_index]->height    = height;
	m_hres->custom[m_index]->graphr    = graphr;
	m_hres->custom[m_index]->graphg    = graphg;
	m_hres->custom[m_index]->graphb    = graphb;
	m_hres->custom[m_index]->label     = label;
	m_hres->custom[m_index]->labelsize = labelsize;
	m_hres->custom[m_index]->labelr    = labelr;
	m_hres->custom[m_index]->labelg    = labelg;
	m_hres->custom[m_index]->labelb    = labelb;
	m_hres->custom[m_index]->bgcolorr  = bgcolorr;
	m_hres->custom[m_index]->bgcolorg  = bgcolorg;
	m_hres->custom[m_index]->bgcolorb  = bgcolorb;
	m_hres->custom[m_index]->tooltip   = tooltip;
}
