#pragma once

#include <QtGui/QLineEdit>

class Dialog : public QWidget
{
   Q_OBJECT

public:
   Dialog();
   ~Dialog();

private slots:
   void textChanged( const QString & text );

private:
   QLineEdit *stringEdit;
   QLineEdit *rxEdit;
   QLineEdit *outEdit;
};
