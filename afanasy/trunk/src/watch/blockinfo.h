#pragma once

#include <QtGui/QPainter>

#include "../include/afjob.h"

#include "../libafanasy/name_af.h"

class QStyleOptionViewItem;
class QMenu;

class Item;
class ListItems;

class BlockInfo
{
public:
   BlockInfo( Item * qItem = NULL, int BlockNumer = -1, int JobId = -1);
   ~BlockInfo();

   inline void setItem( Item *qItem) { item = qItem; }
   inline void setBlockNumber( int BlockNumer) { blocknum = BlockNumer;}
   inline void setJobId( int JobId) { jobid = JobId;}

   bool update( const af::BlockData* block, int type);
   void paint( QPainter * painter, const QStyleOptionViewItem &option,
               int x, int y, int w,
               bool compact_display = false,
               const QColor * backcolor = NULL)
            const;

   inline int getTasksRunning()         const { return tasksrunning;       }
   inline int getPercentage()           const { return percentage;         }
   inline int getErrorsAvoidHost()      const { return errors_avoidhost;   }
   inline int getErrorsRetries()        const { return errors_retries;     }
   inline int getErrorsTaskSameHost()   const { return errors_tasksamehost;}
   inline uint32_t getTasksMaxRunTime() const { return tasksmaxruntime;    }

   void setName( const QString & str) { name = str;}
   inline const QString & getName() const { return name;}

   const QString generateToolTip() const;

   static const int Height;
   static const int HeightCompact;

   void generateMenu( int id_block, QMenu * menu, QWidget * qwidget);

   void blockAction( int id_block, int id_action, ListItems * listitems);

private:
   uint32_t state;

   QString name;
   QString service;
   QString str_runtime;
   QString str_progress;
   QString str_percent;
   QString str_compact;
   QString str_properties;
   QString str_avoiderrors;

   int tasksnum;
   int tasksready;
   int tasksrunning;
   int tasksdone;
   int taskserror;
   int percentage;
   uint32_t taskssumruntime;

   int avoidhostsnum;
   int errorhostsnum;

   int errors_retries;
   int errors_avoidhost;
   int errors_tasksamehost;
   uint32_t tasksmaxruntime;

   uint8_t  progress_done[AFJOB::PROGRESS_BYTES];
   uint8_t  progress_running[AFJOB::PROGRESS_BYTES];

   int     capacity;
   int     maxhosts;
   QString maxhosts_str;
   int     need_memory;
   int     need_power;
   int     need_hdd;
   QString need_properties;

   bool numeric;
   bool varcapacity;
   bool multihost;
   bool multihost_samemaster;
   int  filesize_min;
   int  filesize_max;
   int  capcoeff_min;
   int  capcoeff_max;
   uint8_t  multihost_min;
   uint8_t  multihost_max;
   uint16_t multihost_waitmax;
   uint16_t multihost_waitsrv;

   QString dependmask;
   QString tasksdependmask;
   QString hostsmask;
   QString hostsmask_exclude;

   QString depends;

private:
   Item * item;
   int blocknum;
   int jobid;

   void drawProgress(
         QPainter * painter,
         int posx, int posy, int width, int height,
         const uint8_t * data_a, const uint8_t * data_b,
         const QColor * backcolor = NULL
      ) const;
   void stdOutFlags( char* data, int size) const;
   void refresh();

   const QPixmap * icon_large;
   const QPixmap * icon_small;
};
