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

void GetResources(af::HostRes & o_hres, bool i_verbose)
{
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
}
