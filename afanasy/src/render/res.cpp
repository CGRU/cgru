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

	GetGPUInfo_NVIDIA(o_hres, i_verbose);
}
