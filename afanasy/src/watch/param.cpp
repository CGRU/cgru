#include "param.h"

#include "../libafqt/qenvironment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

Param::Param(int i_type, const QString & i_name, const QString & i_label, const QString & i_tip, int i_min, int i_max):
	type(i_type),
	name(i_name),
	label(i_label),
	tip(i_tip),
	min(i_min),
	max(i_max)
{
}

Param::~Param()
{
}

