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
