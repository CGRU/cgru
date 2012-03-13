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
