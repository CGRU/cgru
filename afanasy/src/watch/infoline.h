#pragma once

#include "../libafqt/qenvironment.h"

#include <QtGui/QLineEdit>

class InfoLine : public QLineEdit
{
public:
   InfoLine( QWidget *parent);
   ~InfoLine();

   void displayInfo(    const QString &message );
   void displayWarning( const QString &message );
   void displayError(   const QString &message );

private:

};
