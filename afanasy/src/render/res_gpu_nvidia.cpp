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

/****************************************************************/
/*                                                              */
/*    This feature is supported by SMF Animation Studio, LLC    */
/*      Данная функция реализована при поддержке ООО «СМФ»      */
/*                                                              */
/****************************************************************/

#include "res.h"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "../libafanasy/name_af.h"

#ifdef WINNT
#define popen _popen
#define pclose _pclose
#endif

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

bool GetGPUInfo_NVIDIA(af::HostRes &o_hres, bool i_verbose)
{
	static const int values_size = 5;

	std::array<char, 128> buffer;
	std::string output;

	std::unique_ptr<FILE, decltype(&pclose)> pipe(
		popen(af::Environment::getRenderGPUInfoNvidiaCmd().c_str(), "r"), pclose);
	if (!pipe)
	{
		if (i_verbose)
			AF_WARN << "Failed to run nvidia-smi";
		return false;
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		output += buffer.data();
	}

	output = af::strStrip(output);

	if (i_verbose)
		printf("GetGPUInfo_NVIDIA: \"%s\"\n", output.c_str());

	const std::vector<std::string> values = af::strSplit(af::strSplit(output, "\n")[0], ",");

	if (values.size() < values_size)
	{
		if (i_verbose)
			AF_WARN << "Bad nvidia-smi output: \"" << output << "\"";
		return false;
	}

	// for (int i = 0; i < values.size(); i++) printf("%d: \"%s\"\n", i, af::strStrip(values[i]).c_str());

	o_hres.gpu_gpu_util = af::stoi(values[0]);
	o_hres.gpu_gpu_temp = af::stoi(values[1]);
	o_hres.gpu_mem_total_mb = af::stoi(values[2]);
	o_hres.gpu_mem_used_mb = af::stoi(values[3]);
	o_hres.gpu_string = af::strStrip(values[4]);

	return true;
}
