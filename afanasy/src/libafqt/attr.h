#pragma once

#include <QtCore/QString>

#include "name_afqt.h"

class QByteArray;
class QDomDocument;
class QDomElement;

class afqt::Attr
{
public:
    Attr( const QString & Name, const QString & Label, const QString & initString);
    virtual ~Attr();

    virtual bool readData();
    virtual void writeData();

    virtual bool read( const QDomDocument & doc);
    virtual void write( QByteArray & data);

    const QString & getName()  const { return name; }
    const QString & getLabel() const { return label;}

public:
    QString str;

protected:
    static bool getXMLAttribute(  QDomElement & element, const QString & name, int & value);
    static bool getXMLElement( const QDomDocument & doc, const QString & name, QString & text );

private:
    QString name;
    QString label;
};
