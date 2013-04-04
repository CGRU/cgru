#pragma once

#include "../include/afjob.h"

#include "../libafanasy/name_af.h"

#include <QtGui/QPainter>

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

   inline int getRunningTasksNumber()   const { return runningtasksnumber; }
   inline int getPercentage()           const { return percentage;         }
   inline int getErrorsAvoidHost()      const { return errors_avoidhost;   }
   inline int getErrorsRetries()        const { return errors_retries;     }
   inline int getErrorsTaskSameHost()   const { return errors_tasksamehost;}
   inline uint32_t getTasksMaxRunTime() const { return tasksmaxruntime;    }

   void setName( const QString & str) { name = str;}
   inline const QString & getName() const { return name;}

   static const int Height;
   static const int HeightCompact;

   void generateMenu( int id_block, QMenu * menu, QWidget * qwidget, QMenu * submenu = NULL);

   bool blockAction( std::ostringstream & i_str, int id_block, const QString & i_action, ListItems * listitems) const;

private:
   uint32_t state;

   int runningtasksnumber;

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
   int tasksdone;
   int taskserror;
   int percentage;
   long long taskssumruntime;

   bool numeric;              ///< Whether the block is numeric.
	bool nonsequential;
   bool varcapacity;
   bool multihost;
   bool multihost_samemaster;

   long long frame_first;     ///< First tasks frame.
   long long frame_last;      ///< Last tasks frame.
   long long frame_pertask;   ///< Tasks frames per task.
   long long frame_inc;       ///< Tasks frames increment.

   int avoidhostsnum;
   int errorhostsnum;

   int errors_retries;
   int errors_avoidhost;
   int errors_tasksamehost;
   int errors_forgivetime;
   uint32_t tasksmaxruntime;

   char progress[AFJOB::ASCII_PROGRESS_LENGTH];

   int     capacity;
   int     maxrunningtasks;
   int     maxruntasksperhost;
   int     need_memory;
   int     need_power;
   int     need_hdd;
   QString need_properties;

   long long filesize_min;
   long long filesize_max;
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
   bool injobslist;

   void drawProgress(
         QPainter * painter,
         int posx, int posy, int width, int height,
         const QColor * backcolor = NULL
      ) const;
   void stdOutFlags( char* data, int size) const;
   void refresh();

   const QPixmap * icon_large;
   const QPixmap * icon_small;
};
