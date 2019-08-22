#pragma once

#include <QtCore/QString>

class Param
{
public:
	Param(int i_type, const QString & i_name, const QString & i_label, const QString & i_tip, int i_min = -1, int i_max = -1);
	~Param();

	enum Type {
		TInt = 0,
		TStr = 1,
		TReg = 2,
		Time = 3
	};

	const int type;
	const QString name;
	const QString label;
	const QString tip;
	const int min;
	const int max;
};

