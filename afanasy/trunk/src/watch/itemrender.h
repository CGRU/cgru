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
   inline bool isDirty()  const { return dirty; }
   inline bool hasTasks() const { return tasks.size() > 0; }

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
   void deletePlots();

private:
   af::Host    host;
   af::HostRes hres;
   QString     hostAttrs;
   QString     hostUsage;

   int revision;
   QString  version;
   QString  username;
   QString  tasksusers; ///< For sorting and filtering only
   QString  annotation;
   int priority;
   int capacity;

   unsigned time_registered;
   unsigned time_launched;

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
   std::list<const QPixmap*> tasksicons;
   uint32_t taskstartfinishtime;
   QString  taskstartfinishtime_str;

   QString creationTime;

   Plotter plotCpu;
   Plotter plotMem;
   Plotter plotSwp;
   Plotter plotHDD;
   Plotter plotNet;
   Plotter plotIO;
   int plotIO_rn_r, plotIO_rn_g, plotIO_rn_b;
   int plotIO_wn_r, plotIO_wn_g, plotIO_wn_b;
   int plotIO_rh_r, plotIO_rh_g, plotIO_rh_b;
   int plotIO_wh_r, plotIO_wh_g, plotIO_wh_b;

   std::vector<Plotter*> plots;

   int plots_height;

   int update_counter;
};
