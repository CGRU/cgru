#include "param.h"

#include "../libafqt/qenvironment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

Param::Param(const QString & i_name, const QString & i_label, const QString & i_tip):
	m_name(i_name),
	m_label(i_label),
	m_tip(i_tip)
{
}

Param::~Param()
{
}

///////////////////////////////////////////////////////
///////////////////   Param_Types   ///////////////////
///////////////////////////////////////////////////////

Param_Int::Param_Int(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip,
		int i_default,
		int i_min, int i_max):
	Param(i_name, i_label, i_tip)
{
}
Param_Int::~Param_Int(){}

Param_Str::Param_Str(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip):
	Param(i_name, i_label, i_tip)
{
}
Param_Str::~Param_Str(){}

