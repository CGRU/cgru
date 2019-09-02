#pragma once

#include "name_afqt.h"

#include <QtCore/QString>

class QByteArray;
class QDomDocument;

class afqt::Attr
{
public:
	Attr();
	Attr(const Attr & i_other);
	Attr(const QString & i_name, const QString & i_label, const QString & i_init);
	virtual ~Attr();

	virtual const QString v_writeData();

	virtual bool v_read( const JSON & i_obj);
	virtual void v_write( QByteArray & o_data);

    const QString & getName()  const { return name; }
    const QString & getLabel() const { return label;}

public:
    QString str;

private:
    QString name;
    QString label;
};

class afqt::AttrNumber: public Attr
{
public:
	AttrNumber();
	AttrNumber(const AttrNumber & i_other);
	AttrNumber(const QString & i_name, int i_init);
	AttrNumber(const QString & i_name, const QString & i_label, int i_init);
	~AttrNumber();

	virtual bool v_read( const JSON & i_obj);
	const QString v_writeData();

public:
   int n;

private:
};

