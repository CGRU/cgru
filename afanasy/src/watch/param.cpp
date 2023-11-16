#include "param.h"

#include <QDateTime>
#include <QInputDialog>

#include "../libafanasy/regexp.h"

#include "../libafqt/qenvironment.h"

#if QT_VERSION < 0x051500
#define Qt_QString QString
#else
#define Qt_QString Qt
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

Param::Param(EType i_type, Item::EType i_itemtype, const QString & i_name, const QString & i_label, const QString & i_tip, int i_min, int i_max):
	type(i_type),
	itemtype(i_itemtype),
	name(i_name),
	label(i_label),
	tip(i_tip),
	m_min(i_min),
	m_max(i_max)
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
	case tsep:
	{
		AF_ERR << "Param::varToQStr(): This parameters separator.";
		break;
	}
	case TNum:
	case TMiB:
	case TGiB:
	{
		int64_t value = i_var.toLongLong();
		is_default = (value == -1);

		if ((value > 0) && (type != TNum))
		{
			if (type == TMiB)
				value *= (1<<20);
			else
				value *= (1<<30);
			str = afqt::stoq(af::toKMG(value, 1<<10, " ") + "B");
		}
		else
			str = QString("%1").arg(value);

		break;
	}
	case TMeg:
	{
		int64_t value = i_var.toLongLong();
		is_default = (value == -1);

		if (value > 0)
		{
			value *= 1000000;
			str = afqt::stoq(af::toKMG(value, 1000, " "));
		}
		else
			str = QString("%1").arg(value);

		break;
	}
	case TStr:
	case TREx:
	{
		str = i_var.toString();
		is_default = str.isEmpty();
		break;
	}
	case TMSI:
	{
		QMapIterator<QString, QVariant> it(i_var.toMap());
		while (it.hasNext())
		{
			it.next();
			if (str.size()) str += ",";
			str += QString("<b>%1</b>:%2").arg(it.key()).arg(it.value().toInt());
		}
		is_default = str.isEmpty();
		break;
	}
	case TMSS:
	{
		QMapIterator<QString, QVariant> it(i_var.toMap());
		while (it.hasNext())
		{
			it.next();
			if (str.size()) str += ",";
			str += QString("<b>%1</b>:%2").arg(it.key()).arg(it.value().toString());
		}
		is_default = str.isEmpty();
		break;
	}
	case Time:
	{
		int64_t value = i_var.toLongLong();
		if (value <= 0)
		{
			is_default = true;
			str = "";
		}
		else
			str = QDateTime::fromSecsSinceEpoch(value).toString(afqt::QEnvironment::getDateTimeFormat());
		break;
	}
	case THrs:
	{
		int64_t value = i_var.toLongLong();
		if (value <= 0)
		{
			is_default = true;
			str = "";
		}
		else
			str = afqt::stoq(af::time2strHMS(value, true));
		break;
	}
	default:
		AF_ERR << "Param::varToQStr(): Unknown parameter '" << name.toUtf8().data() << "' type: " << type;
	}

	if (o_default)
		*o_default = is_default;

	return str;
}

bool Param::getInputDialog(const QVariant & i_var, QString & o_str, QWidget * i_parent) const
{
	bool ok = false;
	switch(type)
	{
	case tsep:
	{
		AF_ERR << "Param::getInputDialog(): This parameters separator.";
		break;
	}
	case TNum:
	case TGiB:
	{
		int64_t current = i_var.toLongLong();
		QString _tip = tip;
		if (type == TGiB)
			_tip += "\nEnter number of Gigabytes";
		int64_t value = QInputDialog::getInt(i_parent, label, _tip, current, m_min, m_max, 1, &ok);
		if (ok)
			o_str = QString("%1").arg(value);
		break;
	}
	case TStr:
	{
		QString current = i_var.toString();
		QString value = QInputDialog::getText(i_parent, label, tip, QLineEdit::Normal, current, &ok);
		if (ok)
			o_str = QString("\"%1\"").arg(value);
		break;
	}
	case TMSI:
	{
		QString current;
		QMapIterator<QString, QVariant> it(i_var.toMap());
		while (it.hasNext())
		{
			it.next();
			if (current.size()) current += ",";
			current += QString("%1:%2").arg(it.key()).arg(it.value().toInt());
		}
		QString _tip = tip + "\nExamples:\nsim:10\nsim:10,render:-100";
		QString value = QInputDialog::getText(i_parent, label, _tip, QLineEdit::Normal, current, &ok);
		if (false == ok)
			break;

		o_str.clear();
		QStringList list = value.split(",", Qt_QString::SkipEmptyParts);
		for (int i = 0; i < list.size(); i++)
		{
			QStringList pair = list[i].split(":", Qt_QString::SkipEmptyParts);
			if (pair.size() != 2)
			{
				o_str = "Invalid pools string.";
				ok = false;
				break;
			}

			if (i) o_str += ",";
			o_str += QString("\"%1\":%2").arg(pair[0]).arg(pair[1]);
		}

		if (false == ok)
			break;

		o_str = QString("{%1}").arg(o_str);

		break;
	}
	case TMSS:
	{
		QString current;
		QMapIterator<QString, QVariant> it(i_var.toMap());
		while (it.hasNext())
		{
			it.next();
			if (current.size()) current += ",";
			current += QString("%1:%2").arg(it.key()).arg(it.value().toString());
		}
		QString _tip = tip + "\nExamples:\nNAME_A:TEXT_A\nNAME_A:TEXT_A,NAME_B:TEXT_B";
		QString value = QInputDialog::getText(i_parent, label, _tip, QLineEdit::Normal, current, &ok);
		if (false == ok)
			break;

		o_str.clear();
		QStringList list = value.split(",", Qt_QString::SkipEmptyParts);
		for (int i = 0; i < list.size(); i++)
		{
			QStringList pair = list[i].split(":", Qt_QString::SkipEmptyParts);
			if (pair.size() != 2)
			{
				o_str = "Invalid environment string.";
				ok = false;
				break;
			}

			if (i) o_str += ",";
			o_str += QString("\"%1\":\"%2\"").arg(pair[0]).arg(pair[1]);
		}

		if (false == ok)
			break;

		o_str = QString("{%1}").arg(o_str);

		break;
	}
	case TREx:
	{
		QString current = i_var.toString();
		QString value = QInputDialog::getText(i_parent, label, tip, QLineEdit::Normal, current, &ok);
		if (ok)
		{
			std::string err;
			if (false == af::RegExp::Validate(afqt::qtos(value), &err))
			{
				ok = false;
				o_str = afqt::stoq(err);
			}
			else
				o_str = QString("\"%1\"").arg(value);
		}
		break;
	}
	case Time:
	{
		int64_t time = i_var.toLongLong();
		QDateTime qdt;
		if (time > 0)
			qdt = QDateTime::fromSecsSinceEpoch(time);
		else
			qdt = QDateTime::currentDateTime();

		QString _tip = tip + "\n" + afqt::QEnvironment::getDateTimeFormat();
		QString current = qdt.toString(afqt::QEnvironment::getDateTimeFormat());
		QString value = QInputDialog::getText(i_parent, label, _tip, QLineEdit::Normal, current, &ok);
		if (ok)
		{
			if ((value.size() == 0) || (value == "0"))
			{
				o_str = "0";
				break;
			}

			QDateTime qdt = QDateTime::fromString(value, afqt::QEnvironment::getDateTimeFormat());
			if (false == qdt.isValid())
			{
				o_str = "Invalid date string.";
				ok = false;
			}
			else
				o_str = QString("%1").arg(qdt.toSecsSinceEpoch());
		}
		break;
	}
	case THrs:
	{
		int64_t time = i_var.toLongLong();

		QString _tip = tip + "\nEnter number of hours";
		double current = double(time) / (60.0 * 60.0);
		double value = QInputDialog::getDouble(i_parent, label, _tip, current, -1, 1<<30, 2, &ok);

		if (ok)
		{
			if (value <= 0)
				o_str = "0";
			else
				o_str = QString("%1").arg(int(value * (60.0 * 60.0)));
		}
		break;
	}
	case TMiB:
	{
		QString _tip = tip + "\nEnter number of Gigabytes";
		double current = double(i_var.toLongLong()) / (1<<10);
		double value = QInputDialog::getDouble(i_parent, label, _tip, current, -1, 1<<30, 2, &ok);

		if (ok)
		{
			if (value == 0)
				o_str = "0";
			else if (value < 0)
				o_str = "-1";
			else
				o_str = QString("%1").arg(int(value * (1<<10)));
		}
		break;
	}
	case TMeg:
	{
		QString _tip = tip + "\nEnter a float Giga";
		double current = double(i_var.toLongLong()) / 1000.0;
		double value = QInputDialog::getDouble(i_parent, label, _tip, current, -1, 1<<30, 2, &ok);

		if (ok)
		{
			if (value == 0)
				o_str = "0";
			else if (value < 0)
				o_str = "-1";
			else
				o_str = QString("%1").arg(int(value * 1000));
		}
		break;
	}
	default:
		AF_ERR << "Unknown parameter '" << name.toUtf8().data() << "' type: " << type;
	}

	return ok;
}
