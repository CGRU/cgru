#pragma once

#include <QtGui/QAction>

class ActionId : public QAction
{

Q_OBJECT

public:
   ActionId( int ID, const QString & text, QObject * parent );

signals:
   void triggeredId( int);

private slots:
   void triggeredId_Slot();

private:
   int id;
};

class ActionIdId : public QAction
{

Q_OBJECT

public:
   ActionIdId ( int ID_1, int ID_2, const QString & text, QObject * parent );

signals:
   void triggeredId( int, int);

private slots:
   void triggeredId_Slot();

private:
   int id_1;
   int id_2;
};

class ActionString : public QAction
{

Q_OBJECT

public:
   ActionString( const QString & i_string, const QString & i_text, QObject * i_parent );

signals:
   void triggeredString( QString );

private slots:
   void triggeredString_Slot();

private:
   QString m_string;
};
