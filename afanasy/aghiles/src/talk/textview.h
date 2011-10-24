#pragma once

#include <QtGui/QTextEdit>

class TextView : public QTextEdit
{
Q_OBJECT
public:
   TextView( QWidget * parent, const QString & UserName);
   ~TextView();

   void appendMessage( const QString &from, const QString &message);

signals:
   void activated();

private:
   QTextCharFormat formatTime;
   QTextCharFormat formatName;
   QTextCharFormat formatText;

   QString username;
};
