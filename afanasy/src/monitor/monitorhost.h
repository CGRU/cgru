#pragma once

#include "../libafanasy/monitor.h"

class MonitorHost: public af::Monitor
{
public:
   MonitorHost();
   ~MonitorHost();

   inline void setId( int new_id) { m_id = new_id;}

private:
};
