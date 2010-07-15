#pragma once

#include "../libafanasy/render.h"

#include "itemnode.h"
#include "plotter.h"

class ItemRender : public ItemNode
{
public:
   ItemRender( af::Render *render);
   ~ItemRender();

   void updateValues( af::Node *node, int type);

   inline const QString & getUserName()   const { return username;   }
   inline const QString & getAnnotation() const { return annotation; }
   inline int getPriority() const { return priority;  }
   inline int getCapacity() const { return capacity;  }

   bool setSortType(   int type );
   bool setFilterType( int type );

   inline bool isOnline() const { return online;}
   inline bool isBusy()   const { return busy;  }
   inline bool isnimby()  const { return nimby; }
   inline bool isNIMBY()  const { return NIMBY; }

   bool calcHeight();

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
   static const int HeightHost;
   static const int HeightAnnotation;
   static const int HeightTask;
   static const int HeightOffline;

private:
   void deleteTasks();

private:
   af::Host    host;
   af::HostRes hres;
   QString     hostAttrs;
   QString     hostUsage;

   QString  username;
   QString  annotation;
   int priority;
   int capacity;

   QString  address_str;      ///< Client address

   bool online;
   bool busy;
   bool NIMBY;
   bool nimby;
   bool dirty;

   int capacity_used;
   QString capacity_usage;

   QString  state;
   std::list<af::TaskExec*> tasks;
   uint32_t taskstartfinishtime;
   QString  taskstartfinishtime_str;

   QString creationTime;

   Plotter plotCpu;
   Plotter plotMem;
   Plotter plotSwp;
   Plotter plotHDD;
   Plotter plotNet;
};
