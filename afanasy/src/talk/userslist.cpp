#include "userslist.h"

#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

UsersList::UsersList( QWidget * parent, const QString & UserName):
   QListWidget( parent),
   username( UserName)
{
   setSelectionMode( QAbstractItemView::MultiSelection);
   connect( this, SIGNAL( itemDoubleClicked( QListWidgetItem *)), this, SLOT( selectOnly( QListWidgetItem *)));
   connect( this, SIGNAL( itemSelectionChanged()               ), this, SLOT( selectOwner()                 ));
   setFixedWidth( 99);
}

UsersList::~UsersList()
{
}
/*
void UsersList::focusInEvent( QFocusEvent * event)
{
//   UsersList::focusInEvent( event);
   emit activated();
}
*/
void UsersList::mousePressEvent( QMouseEvent * event)
{
   QListWidget::mousePressEvent( event);
   switch( event->button())
   {
      case Qt::MidButton:
         if( currentItem()->text() != username) selectAll( false == currentItem()->isSelected());
         break;
      case Qt::LeftButton:
      case Qt::RightButton:
      case Qt::XButton1:
      case Qt::XButton2:
      case Qt::NoButton:
         break;
   }
}

void UsersList::selectAll( bool select, const QListWidgetItem * skip)
{
   for( int i = 0; i < count(); i++)
      if((item(i)->text() != username) && (skip != item(i)))
         item(i)->setSelected( select);
}

void UsersList::selectOnly( QListWidgetItem * item)
{
   if( item->text() == username) return;
   item->setSelected( true);
   selectAll( false, item);
}

void UsersList::selectOwner()
{
   for( int i = 0; i < count(); i++)
      if((item(i)->text() == username) && (false == item(i)->isSelected()))
         item(i)->setSelected( true);
}

void UsersList::updateOnline( const QStringList & users, const QStringList & hosts)
{
//printf("UsersList::updateOnline:\n");for( int i = 0; i < users.size(); i++) printf("%s\n", users[i].toUtf8().data());
   for( int i = 0; i < count(); i++)
      if( false == users.contains( item(i)->text()))
         delete takeItem(i);

   for( int i = 0; i < users.size(); i++)
      if( findItems( users[i], Qt::MatchExactly).empty()) addItem( users[i]);

   selectOwner();
}
