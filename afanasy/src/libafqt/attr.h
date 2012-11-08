#pragma once

#include "name_afqt.h"

#include <QtCore/QString>

class QByteArray;
class QDomDocument;
class QDomElement;

class afqt::Attr
{
public:
    Attr( const QString & Name, const QString & Label, const QString & initString);
    virtual ~Attr();

	virtual const QString v_writeData();

	virtual bool v_read( const JSON & i_obj);
	virtual void v_write( QByteArray & o_data);

    const QString & getName()  const { return name; }
    const QString & getLabel() const { return label;}

public:
    QString str;

/*protected:
    static bool getXMLAttribute(  QDomElement & element, const QString & name, int & value);
    static bool getXMLElement( const QDomDocument & doc, const QString & name, QString & text );
*/
private:
    QString name;
    QString label;
};
