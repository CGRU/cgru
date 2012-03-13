#pragma once

#include <monitor.h>

class MonitorHost: public af::Monitor
{
public:
   MonitorHost();
   ~MonitorHost();

   inline void setId( int new_id) { id = new_id;}

private:
};
