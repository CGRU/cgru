#pragma once

#include "../libafanasy/job.h"

#include "itemnode.h"
#include "blockinfo.h"

class ItemJob : public ItemNode
{
public:
   ItemJob( af::Job *job);
   ~ItemJob();

   void updateValues( af::Node *node, int type);

   inline int getErrorRetries(         int block = 0 ) const
      {if(block<blocksnum )return blockinfo[ block].getErrorsRetries();       else return 0; }
   inline int getErrorsAvoidHost(      int block = 0 ) const
      {if(block<blocksnum )return blockinfo[ block].getErrorsAvoidHost();     else return 0; }
   inline int getErrorsTaskSameHost(   int block = 0 ) const
      {if(block<blocksnum )return blockinfo[ block].getErrorsTaskSameHost();  else return 0; }
   inline uint32_t getTasksMaxRunTime( int block = 0 ) const
      {if(block<blocksnum )return blockinfo[ block].getTasksMaxRunTime();     else return 0; }

   int priority;
   int maxrunningtasks;
   int maxruntasksperhost;
   uint32_t state;
   int64_t time_creation;
   int64_t time_started;
   int64_t time_wait;
   int64_t time_done;
   uint32_t time_run;
   int lifetime;
   QString annotation;
   QString hostname;
   QString username;
   QString hostsmask;
   QString hostsmask_exclude;
   QString dependmask;
   QString dependmask_global;
   QString need_os;
   QString need_properties;
   QString cmd_pre;
   QString cmd_post;
   QString description;

   inline int getBlocksNum() const { return blocksnum;}
   inline int getBlockPercent( int block ) const
      { if( block < blocksnum ) return blockinfo[block].getPercentage(); else return 0;}

   bool setSortType(   int type );
   bool setFilterType( int type );

   void generateMenu( int id_block, QMenu * menu, QWidget * qwidget);

   bool blockAction( std::ostringstream & i_str, int id_block, const QString & i_action, ListItems * listitems) const;

   inline const QString & getBlockName( int num) const { return blockinfo[num].getName();}

   bool calcHeight();

protected:
   void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
   int blocksnum;
   bool compact_display;

   int tasksdone_job;
   int taskserror_job;
   int taskssumruntime_job;

   QString properties;

   QStringList blocksinfo;
   QString user_time;

   QString runningTime;

   int     num_runningtasks;
   QString num_runningtasks_str;

private:

   static const int Height;
   static const int HeightAnnotation;
   int block_height;
   BlockInfo * blockinfo;
};
