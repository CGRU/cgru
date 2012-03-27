#pragma once

#include "../libafanasy/name_af.h"

#include "wnd.h"

class WndTask : public Wnd
{
public:
	WndTask( const QString & Name, af::Msg * msg = NULL);
	~WndTask();
};
