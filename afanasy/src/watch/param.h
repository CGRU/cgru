#pragma once

#include "watch.h"

#include <QtCore/QString>
#include <QtCore/QVariant>

class Param
{
public:
	enum EType {
		TNum = 0,
		TStr = 1,
		TREx = 2,
		Time = 3,
		THrs = 4
	};

	Param(EType i_type, const QString & i_name, const QString & i_label, const QString & i_tip, int i_min = -1, int i_max = -1);
	~Param();

	const EType type;
	const QString name;
	const QString label;
	const QString tip;
	const int min;
	const int max;


	// Convert variable to string.
	// Default is true if value is default,
	// as default parameters can be hidden.
	const QString varToQStr(const QVariant & i_var, bool * o_default = NULL) const;


	// Get value from input dialog.
	bool getInputDialog(const QVariant & i_var, QString & o_str) const;
};

