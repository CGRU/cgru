#pragma once

#include <QtCore/QString>

class Param
{
public:
	Param(const QString & i_name, const QString & i_label, const QString & i_tip);
	virtual ~Param();

protected:

private:
	QString m_name;
	QString m_label;
	QString m_tip;
};

class Param_Int: public Param
{
public:
	Param_Int(const QString & i_name, const QString & i_label, const QString & i_tip, int i_default, int i_min = -1, int i_max = -1);
	virtual ~Param_Int();

protected:

private:
};

class Param_Str: public Param
{
public:
	Param_Str(const QString & i_name, const QString & i_label, const QString & i_tip);
	virtual ~Param_Str();

protected:

private:
};

