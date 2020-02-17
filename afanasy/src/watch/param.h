#pragma once

#include "item.h"
#include "watch.h"

#include <QtCore/QString>
#include <QtCore/QVariant>

class Param
{
public:
	enum EType {
		tsep = 0,
		TNum = 1,
		TStr = 2,
		TREx = 3,
		Time = 4,
		THrs = 5
	};

	Param(EType i_type, Item::EType i_itemtype, const QString & i_name, const QString & i_label, const QString & i_tip, int i_min = -1, int i_max = -1);
	~Param();

	const EType type;
	const Item::EType itemtype;
	const QString name;
	const QString label;
	const QString tip;
	const int min;
	const int max;

	bool isSeparator() const {return type == tsep;}

	// Convert variable to string.
	// Default is true if value is default,
	// as default parameters can be hidden.
	const QString varToQStr(const QVariant & i_var, bool * o_default = NULL) const;


	// Get value from input dialog.
	bool getInputDialog(const QVariant & i_var, QString & o_str, QWidget * i_parent) const;
};
