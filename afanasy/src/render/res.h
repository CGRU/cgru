#pragma once

#include "../libafanasy/hostres.h"

void GetResources(af::HostRes & hres, bool verbose = false);

void GetResources_LINUX(af::HostRes & hres, bool verbose = false);
void GetResources_MACOSX(af::HostRes & hres, bool verbose = false);
void GetResources_WINDOWS(af::HostRes & hres, bool verbose = false);

bool GetGPUInfo_NVIDIA(af::HostRes & o_hres, bool i_verbose = false);
