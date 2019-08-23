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

const QString Param::varToQStr(const QVariant & i_var, bool * o_default) const
{
	bool is_default = false;
	QString str;

	switch(type)
	{
	case TInt:
	{
		int value = i_var.toInt();
		str = QString("%1").arg(value);
		is_default = (value == -1);
		break;
	}
	case TStr:
	{
		str = i_var.toString();
		is_default = str.isEmpty();
		break;
	}
	default:
		AF_ERR << "Unknown parameter type: " << type;
	}

	if (o_default)
		*o_default = is_default;

	return str;
}

