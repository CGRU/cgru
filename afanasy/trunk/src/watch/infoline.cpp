#include "infoline.h"

InfoLine::InfoLine( QWidget *parent):
   QLineEdit( parent)
{
   setReadOnly( true);
   setFrame( false);
}

InfoLine::~InfoLine()
{
}

void InfoLine::displayInfo( const QString &message)
{
   setText( message);
}

void InfoLine::displayWarning( const QString &message)
{
   setText( "WARNING: " + message);
}

void InfoLine::displayError( const QString &message)
{
   setText( "ERROR: " + message);
}
