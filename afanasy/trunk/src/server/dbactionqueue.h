#pragma once

#include <QtSql/QSqlDatabase>
#include <QtCore/QStringList>

#include "../libafanasy/name_af.h"

#include "../libafsql/name_afsql.h"

#include "afqueue.h"
#include "afqueueitem.h"

class Queries: public QStringList, public AfQueueItem {};

/// Simple FIFO database action queue
class DBActionQueue : public AfQueue
{
public:
   DBActionQueue( const QString & QueueName);
   ~DBActionQueue();

   void addItem(    const afsql::DBItem * item);
   void delItem(    const afsql::DBItem * item);
   void updateItem( const afsql::DBItem * item, int attr = -1);

   void quit();

/// Push queries to queue back.
   inline bool pushQueries( Queries * queries) { if(db->isOpen()) return push( queries); else return false;}

protected:
   void processItem( AfQueueItem* item) const;

   QSqlDatabase * db;
};
