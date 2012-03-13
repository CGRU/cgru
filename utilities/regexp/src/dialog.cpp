#include "dialog.h"

#include <QtCore/QRegExp>
#include <QtGui/QBoxLayout>

Dialog::Dialog()
{
   setWindowTitle( "Qt RegExp checker:");

   QVBoxLayout *vlayout = new QVBoxLayout( this);

   stringEdit  = new QLineEdit( "render01", this);
   rxEdit      = new QLineEdit( "((render|workstation)0[1-4]{1,})", this);
   outEdit     = new QLineEdit( "render01-render04 or workstation01-workstation04", this);

   vlayout->addWidget( stringEdit);
   vlayout->addWidget( rxEdit);
   vlayout->addWidget( outEdit);

   outEdit->setReadOnly(true);
   outEdit->setFocusPolicy(Qt::NoFocus);
   outEdit->setAutoFillBackground( true);

   connect( stringEdit, SIGNAL( textChanged( const QString &)), this, SLOT( textChanged( const QString &)));
   connect( rxEdit,     SIGNAL( textChanged( const QString &)), this, SLOT( textChanged( const QString &)));

   resize( 500 , 100);
}

Dialog::~Dialog()
{
}

void Dialog::textChanged( const QString & text )
{
   outEdit->setText( stringEdit->text() + " - " + rxEdit->text());

   QRegExp rx( rxEdit->text(), Qt::CaseInsensitive);
   QString str( stringEdit->text());
   if( rx.isValid() == false )
   {
      outEdit->setText( QString("ERROR: %1").arg(rx.errorString()).toUtf8().data());
      return;
   }
   if( rx.exactMatch( str) )
   {
      outEdit->setText( "MATCH");
   }
   else
   {
      outEdit->setText( "NOT MATCH");
   }
}
