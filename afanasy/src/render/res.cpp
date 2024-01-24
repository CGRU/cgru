/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

#include "res.h"

#include "../include/afpynames.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/pymodule.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

af::PyModule * PyMod_GetCPUTemperature = NULL;
PyObject * PyFunc_GetCPUTemperature = NULL;

af::PyModule * PyMod_GetHWInfo = NULL;
PyObject * PyFunc_GetHWInfo = NULL;

// Haders:
void InitResources();

void GetResources_LINUX(af::HostRes & hres, bool verbose = false);
void GetResources_MACOSX(af::HostRes & hres, bool verbose = false);
void GetResources_WINDOWS(af::HostRes & hres, bool verbose = false);

bool GetGPUInfo_NVIDIA(af::HostRes & o_hres, bool i_verbose = false);

int getCPUTemperature();

const std::string getHWInfo();

// Definitions:
void GetResources(af::HostRes & o_hres, bool i_verbose)
{
	static int counter = 0;

	if (counter == 0)
		InitResources();

#ifdef LINUX
	GetResources_LINUX(o_hres, i_verbose);
#endif

#ifdef MACOSX
	GetResources_MACOSX(o_hres, i_verbose);
#endif

#ifdef WINNT
	GetResources_WINDOWS(o_hres, i_verbose);
#endif

	if (GetGPUInfo_NVIDIA(o_hres, i_verbose) == false)
	{
		o_hres.gpu_gpu_util     = 0;
		o_hres.gpu_gpu_temp     = 0;
		o_hres.gpu_mem_used_mb  = 0;
		o_hres.gpu_mem_total_mb = 0;
		o_hres.gpu_string.clear();
	}

	o_hres.cpu_temp = getCPUTemperature();

	o_hres.hw_info = getHWInfo();

	counter++;
}

void InitResources()
{
	PyMod_GetCPUTemperature = new af::PyModule();
	if (PyMod_GetCPUTemperature->init(AFPYNAMES::RES_CLASSESDIR,af::Environment::getRenderCPUTemperatureMod()))
		PyFunc_GetCPUTemperature = PyMod_GetCPUTemperature->getFunction(af::Environment::getRenderCPUTemperatureMod());

	PyMod_GetHWInfo = new af::PyModule();
	if (PyMod_GetHWInfo->init(AFPYNAMES::RES_CLASSESDIR, af::Environment::getRenderHWInfoMod()))
		PyFunc_GetHWInfo = PyMod_GetHWInfo->getFunction(af::Environment::getRenderHWInfoMod());
}

void FreeResources()
{
	delete PyMod_GetCPUTemperature;
	delete PyMod_GetHWInfo;
}

int getCPUTemperature()
{
	if (PyFunc_GetCPUTemperature == NULL)
		return 0;

	PyObject * pResult = PyObject_CallObject(PyFunc_GetCPUTemperature, NULL);
	if (pResult == NULL)
	{
		if (PyErr_Occurred())
			PyErr_Print();
		return 0;
	}

	if (true != PyLong_Check(pResult))
	{
		AF_ERR << "Return object type is not an integer.";
		return 0;
	}

	int result = PyLong_AsLong(pResult);

	Py_DECREF(pResult);

	return result;
}

const std::string getHWInfo()
{
	std::string hw_info;

	if (PyFunc_GetHWInfo == NULL)
		return hw_info;

	PyObject * pResult = PyObject_CallObject(PyFunc_GetHWInfo, NULL);
	if (pResult == NULL)
	{
		if (PyErr_Occurred())
			PyErr_Print();
		return hw_info;
	}

	if (false == af::PyGetString(pResult, hw_info))
	{
		AF_ERR << "Return object type is not a string.";
		Py_DECREF(pResult);
		return hw_info;
	}

	Py_DECREF(pResult);

	return hw_info;
}
