#pragma once

#include <QtGui/QListWidget>

class UsersList : public QListWidget
{
Q_OBJECT
public:
   UsersList( QWidget * parent, const QString & UserName);
   ~UsersList();

   void updateOnline( const QStringList & users, const QStringList & hosts);

signals:
   void activated();

protected:
   void mousePressEvent( QMouseEvent * event);
//   void focusInEvent( QFocusEvent * event);

private slots:
   void selectOnly( QListWidgetItem * item);
   void selectOwner();

private:
   QString username;
   void selectAll( bool select, const QListWidgetItem * skip = NULL);
};
