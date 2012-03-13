#pragma once

#include <job.h>

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
   int maxhosts;
   uint32_t state;
   uint32_t time_creation;
   uint32_t time_started;
   uint32_t time_wait;
   uint32_t time_done;
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

   void blockAction( int id_block, int id_action, ListItems * listitems);

   inline const QString & getBlockName( int num) const { return blockinfo[num].getName();}

protected:
   void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
   int blocksnum;

   int tasksdone_job;
   int taskserror_job;
   int taskssumruntime_job;

   QString properties;

   QStringList blocksinfo;
   QString user_time;

   QString runningTime;

   int     num_hosts;
   QString num_hosts_str;

private:

   static const int Height;
   int block_height;
   BlockInfo * blockinfo;
};
