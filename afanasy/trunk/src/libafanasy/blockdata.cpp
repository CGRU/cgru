#include "blockdata.h"

#include <memory.h>
#include <stdio.h>
#include <string.h>

#include "jobprogress.h"
#include "msg.h"
#include "taskdata.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

BlockData::BlockData()
{
   initDefaults();
   construct();
}

void BlockData::initDefaults()
{
   p_percentage = 0;
   p_errorhostsnum = 0;
   p_avoidhostsnum = 0;
   p_tasksready = 0;
   p_tasksdone = 0;
   p_taskserror = 0;
   p_taskssumruntime = 0;
   jobid = 0;
   blocknum = 0;
   flags = 0;
   frame_first = 0;
   frame_last = 0;
   frame_pertask = 1;
   frame_inc = 1;
   maxrunningtasks = -1;
   tasksmaxruntime = 0;
   capacity = AFJOB::TASK_DEFAULT_CAPACITY;
   need_memory = 0;
   need_power = 0;
   need_hdd = 0;
   errors_retries = -1;
   errors_avoidhost = -1;
   errors_tasksamehost = -1;
   errors_forgivetime = -1;
   filesize_min = -1;
   filesize_max = -1;
   capcoeff_min = 0;
   capcoeff_max = 0;
   multihost_min = 0;
   multihost_max = 0;
   multihost_waitmax = 0;
   multihost_waitsrv = 0;
   parsercoeff = 1;
}

BlockData::BlockData( Msg * msg)
{
   construct();
   read( msg);
}

BlockData::BlockData( int BlockNum, int JobId):
   jobid( JobId),
   blocknum( BlockNum)
{
   construct();
}

void BlockData::construct()
{
   tasksnum = 0;
   tasksdata = NULL;
   runningtasks_counter = 0;

   dependmask.setCaseSensitivity( Qt::CaseSensitive);
   hostsmask.setCaseSensitivity( Qt::CaseInsensitive);
   hostsmask_exclude.setCaseSensitivity( Qt::CaseInsensitive);
   tasksdependmask.setCaseSensitivity( Qt::CaseSensitive);
   need_properties.setCaseSensitivity( Qt::CaseSensitive);
}

bool BlockData::isValid() const
{
   if( tasksnum == 0)
   {
      AFERRAR("BlockData::isValid: #%d block[%s] zero tasks number.\n", blocknum, name.toUtf8().data());
      return false;
   }
   if( isNotNumeric())
   {
      if( tasksdata == NULL)
      {
         AFERRAR("BlockData::isValid: #%d block[%s] tasks data is null on not numeric block.\n", blocknum, name.toUtf8().data());
         return false;
      }
      for( int t = 0; t < tasksnum; t++)
      {
         if(tasksdata[t] == NULL)
         {
            AFERRAR("BlockData::isValid: #%d block[%s] task[%d] data is null on not numeric block.\n", blocknum, name.toUtf8().data(), t);
            return false;
         }
      }
   }
   return true;
}

BlockData::~BlockData()
{
   if( tasksdata)
   {
      for( int t = 0; t < tasksnum; t++)
         if( tasksdata[t]) delete tasksdata[t];
      delete [] tasksdata;
   }
}

void BlockData::readwrite( Msg * msg)
{
//printf("BlockData::readwrite: BEGIN\n");
   switch( msg->type())
   {
   case Msg::TJob:
   case Msg::TJobRegister:
   case Msg::TBlocks:
      rw_int32_t ( frame_first,           msg);
      rw_int32_t ( frame_last,            msg);
      rw_int32_t ( frame_pertask,         msg);
      rw_int32_t ( frame_inc,             msg);
      rw_uint32_t( flags,                 msg);
      if( isNotNumeric()) rw_tasks(       msg);

   case Msg::TBlocksProperties:
      rw_int32_t ( parsercoeff,           msg);
      rw_QString ( tasksname,             msg);
      rw_QString ( parser,                msg);
      rw_QString ( wdir,                  msg);
      rw_QString ( environment,           msg);
      rw_QString ( command,               msg);
      rw_QString ( files,                 msg);
      rw_QString ( cmd_pre,               msg);
      rw_QString ( cmd_post,              msg);
      rw_QString ( multihost_service,     msg);
      rw_QString ( customdata,            msg);

   case Msg::TJobsList:
      rw_uint32_t( flags,                 msg);
      rw_int32_t ( filesize_min,          msg);
      rw_int32_t ( filesize_max,          msg);
      rw_int32_t ( capcoeff_min,          msg);
      rw_int32_t ( capcoeff_max,          msg);
      rw_uint8_t ( multihost_min,         msg);
      rw_uint8_t ( multihost_max,         msg);
      rw_uint16_t( multihost_waitsrv,     msg);
      rw_uint16_t( multihost_waitmax,     msg);
      rw_int32_t ( capacity,              msg);
      rw_int32_t ( maxrunningtasks,       msg);
      rw_int32_t ( need_memory,           msg);
      rw_int32_t ( need_power,            msg);
      rw_int32_t ( need_hdd,              msg);
      rw_QRegExp ( dependmask,            msg);
      rw_QRegExp ( tasksdependmask,       msg);
      rw_QRegExp ( hostsmask,             msg);
      rw_QRegExp ( hostsmask_exclude,     msg);
      rw_QRegExp ( need_properties,       msg);
      rw_QString ( name,                  msg);
      rw_QString ( service,               msg);
      rw_int32_t ( tasksnum,              msg);
      rw_int8_t  ( errors_retries,        msg);
      rw_int8_t  ( errors_avoidhost,      msg);
      rw_int8_t  ( errors_tasksamehost,   msg);
      rw_int32_t ( errors_forgivetime,    msg);
      rw_uint32_t( tasksmaxruntime,       msg);

   case Msg::TBlocksProgress:

      rw_int32_t ( runningtasks_counter,  msg);
      rw_data(   (char*)p_bar_done,       msg, AFJOB::PROGRESS_BYTES);
      rw_data(   (char*)p_bar_running,    msg, AFJOB::PROGRESS_BYTES);
      rw_uint8_t ( p_percentage,          msg);
      rw_int32_t ( p_errorhostsnum,       msg);
      rw_int32_t ( p_avoidhostsnum,       msg);
      rw_int32_t ( p_tasksready,          msg);
      rw_int32_t ( p_tasksdone,           msg);
      rw_int32_t ( p_taskserror,          msg);
      rw_uint32_t( p_taskssumruntime,     msg);

      rw_uint32_t( state,                 msg);
      rw_int32_t ( jobid,                 msg);
      rw_int32_t ( blocknum,              msg);

   break;

   default:
      AFERRAR("BlockData::readwrite: invalid type = %s.\n", Msg::TNAMES[msg->type()]);
   }
//printf("BlockData::readwrite: END\n");
}

void BlockData::rw_tasks( Msg * msg)
{
   if( isNumeric())
   {
      AFERROR("BlockData::rw_tasks: block is numeric.\n");
      return;
   }
   rw_int32_t( tasksnum, msg);
   if( tasksnum < 1)
   {
      AFERRAR("BlockData::rw_tasks: invalid number of tasks = %d.\n", tasksnum);
      return;
   }

   if( msg->isWriting() )
   {
      for( int b = 0; b < tasksnum; b++)
      {
         tasksdata[b]->write( msg);
      }
   }
   else
   {
      tasksdata = new TaskData*[tasksnum];
      for( int b = 0; b < tasksnum; b++)
      {
         tasksdata[b] = createTask( msg);
         if( tasksdata[b] == NULL)
         {
            AFERROR("BlockData::rw_tasks: Can not allocate memory for new task.\n");
            return;
         }
      }
   }
}

TaskData * BlockData::createTask( Msg * msg)
{
//printf("BlockData::createTask:\n");
   return new TaskData( msg);
}

bool BlockData::setCapacity( int value)
{
   if( value > 0)
   {
      capacity = value;
      return true;
   }
   AFERRAR("BlockData::setCapacity: invalid capacity = %d\n", value);
   return false;
}

bool BlockData::setCapacityCoeffMin( int value)
{
   if( canVarCapacity() == false)
   {
      AFERROR("BlockData::setCapacityCoeffMin: Block can't variate capacity.\n");
      return false;
   }
   capcoeff_min = value;
   return true;
}

bool BlockData::setCapacityCoeffMax( int value)
{
   if( canVarCapacity() == false)
   {
      AFERROR("BlockData::setCapacityCoeffMax: Block can't variate capacity.\n");
      return false;
   }
   capcoeff_max = value;
   return true;
}

bool BlockData::setMultiHostMin( int value)
{
   if( isMultiHost() == false)
   {
      AFERROR("BlockData::setMultiHostMin: Block is not multihost.\n");
      return false;
   }
   if( value < 1)
   {
      AFERROR("BlockData::setMultiHostMin: Hosts minimum can't be less than one.\n");
      return false;
   }
   multihost_min = value;
   return true;
}

bool BlockData::setMultiHostMax( int value)
{
   if( isMultiHost() == false)
   {
      AFERROR("BlockData::setMultiHostMax: Block is not multihost.\n");
      return false;
   }
   if( value < multihost_min)
   {
      AFERROR("BlockData::setMultiHostMax: Hosts maximum can't be less than minimum.\n");
      return false;
   }
   multihost_max = value;
   return true;
}

bool BlockData::checkDependMask( const QString & str)
{
   if( dependmask.isEmpty()) return false;
   return dependmask.exactMatch( str);
}
bool BlockData::checkHostsMask( const QString & str)
{
   if( hostsmask.isEmpty()) return true;
   return hostsmask.exactMatch( str );
}
bool BlockData::checkHostsMaskExclude( const QString & str)
{
   if( hostsmask_exclude.isEmpty()) return false;
   return hostsmask_exclude.exactMatch( str);
}
bool BlockData::checkTasksDependMask( const QString & str)
{
   if( tasksdependmask.isEmpty()) return false;
   return tasksdependmask.exactMatch( str);
}
bool BlockData::checkNeedProperties( const QString & str)
{
   if( need_properties.isEmpty()) return true;
   return str.contains( need_properties);
}

bool BlockData::setNumeric( int start, int end, int perTask, int increment)
{
   if( perTask < 1)
   {
      AFERRAR("BlockData::setNumeric(): Frames per task = %d < 1 ( setting to 1).\n", perTask);
      perTask = 1;
   }
   if( tasksdata)
   {
      AFERROR("BlockData::setNumeric(): this block already has tasks.\n");
      return false;
   }
   if( isNumeric())
   {
      AFERROR("BlockData::setNumeric(): this block is already numeric and numbers are set.\n");
      return false;
   }
   if( start > end)
   {
      AFERRAR("BlockData::setNumeric(): start > end ( %d > %d - setting end to %d)\n", start, end, start);
      end = start;
   }
   flags = flags | FNumeric;

   frame_first    = start;
   frame_last     = end;
   frame_pertask  = perTask;
   frame_inc      = increment;

   int numframes = frame_last - frame_first + 1;
   tasksnum = numframes / frame_pertask;
   if((numframes%perTask) != 0) tasksnum++;

   return true;
}

void BlockData::setFramesPerTask( int perTask)
{
   if( isNumeric())
   {
      AFERROR("BlockData::setFramesPerHost: this block is numeric.\n");
      return;
   }
   if( perTask == 0)
   {
      AFERROR("BlockData::setFramesPerHost: Frames per task can't be zero.\n");
      return;
   }
   frame_pertask = perTask;
}

const QString BlockData::genCommand( int num, int *frame_start, int *frame_finish) const
{
   QString str;
   if( num > tasksnum)
   {
      AFERROR("BlockData::getCmd: n > tasksnum.\n");
      return str;
   }
   if( isNumeric())
   {
      int start, end;
      if( genNumbers( start, end, num))
      {
         str = fillNumbers( command, start, end);

         if( frame_start  != NULL) *frame_start  = start;
         if( frame_finish != NULL) *frame_finish = end;
      }
      else
         return str;
   }
   else
   {
      if( command.isEmpty()) return tasksdata[num]->getCommand();
      str = command.arg( tasksdata[num]->getCommand());
   }
   return str;
}

const QString BlockData::genFiles( int num) const
{
   QString str;
   if( num >= tasksnum)
   {
      AFERROR("BlockData::genCmdView: n >= tasksnum.\n");
      return str;
   }
   if( isNumeric())
   {
      if( false == files.isEmpty())
      {
         int start, end;
         if( genNumbers( start, end, num)) str = fillNumbers( files, start, end);
      }
   }
   else
   {
      if( files.isEmpty()) return tasksdata[num]->getFiles();
      str = files.arg( tasksdata[num]->getFiles());
   }
   return str;
}

bool BlockData::genNumbers( int &start, int &end, int num) const
{
   if( num > tasksnum)
   {
      AFERROR("BlockData::genNumbers: n > tasksnum.\n");
      return false;
   }
   start = frame_first + num * frame_pertask;
   end = start + frame_pertask  - 1;
   if( end > frame_last) end = frame_last;

   return true;
}

TaskExec *BlockData::genTask( int num) const
{
   if( num > tasksnum)
   {
      AFERROR("BlockData::genTask: n > tasksnum.\n");
      return NULL;
   }

   int start = -1;
   int end = -1;

   QString command = genCommand( num, &start, &end);

   return new TaskExec(
         genTaskName( num),
         service,
         parser,
         command,
         capacity,
         filesize_min,
         filesize_max,
         genFiles( num),

         start,
         end,
         end-start+1,

         wdir,
         environment,

         jobid,
         blocknum,
         num
      );
}

const QString BlockData::genTaskName( int num) const
{
   if( num > tasksnum)
   {
      AFERROR("BlockData::genTaskName: n > tasksnum.\n");
      return QString("> tasksnum");
   }

   if( isNumeric())
   {
      int start, end;
      genNumbers( start, end, num);
      if( tasksname.isEmpty() == false) return fillNumbers( tasksname, start, end);

      if( frame_pertask == 1 )
         return QString::number( start);
      else
         return QString::number( start) + "-" + QString::number( end);
   }

   if( tasksname.isEmpty()) return tasksdata[num]->getName();

   return tasksname.arg( tasksdata[num]->getName());
}

void BlockData::setStateDependent( bool depend)
{
   if( depend)
   {
      state = state |   AFJOB::STATE_WAITDEP_MASK;
      state = state & (~AFJOB::STATE_READY_MASK);
   }
   else
   {
      state = state & (~AFJOB::STATE_WAITDEP_MASK);
   }
}

int BlockData::calcWeight() const
{
   int weight = sizeof(BlockData);
   if( isNotNumeric() && tasksdata)
      for( int t = 0; t < tasksnum; t++)
         weight += tasksdata[t]->calcWeight();

   weight += weigh(service);
   weight += weigh(parser);
   weight += weigh(dependmask);
   weight += weigh(tasksdependmask);
   weight += weigh(need_properties);
   weight += weigh(hostsmask);
   weight += weigh(hostsmask_exclude);
   weight += weigh(name);
   weight += weigh(wdir);
   weight += weigh(environment);
   weight += weigh(command);
   weight += weigh(files);
   weight += weigh(cmd_pre);
   weight += weigh(cmd_post);
   weight += weigh(customdata);

   return weight;
}

void BlockData::stdOut( bool full) const
{
   printf("BLOCK = \"%s\" ( %s-%s[%d] ), %d tasks.\n",
      name.toUtf8().data(), service.toUtf8().data(), parser.toUtf8().data(), parsercoeff, tasksnum);

                                      printf("Command            = \"%s\"\n",           command.toUtf8().data());
                                      printf("Working Directory  = \"%s\"\n",              wdir.toUtf8().data());
   if( !             files.isEmpty()) printf("Files              = \"%s\"\n",             files.toUtf8().data());
   if( !        dependmask.isEmpty()) printf("Depend Mask        = \"%s\"\n",        dependmask.pattern().toUtf8().data());
   if( !   tasksdependmask.isEmpty()) printf("Tasks Depend Mask  = \"%s\"\n",   tasksdependmask.pattern().toUtf8().data());
   if( !         hostsmask.isEmpty()) printf("Hosts Mask         = \"%s\"\n",         hostsmask.pattern().toUtf8().data());
   if( ! hostsmask_exclude.isEmpty()) printf("Hosts Mask Exclude = \"%s\"\n", hostsmask_exclude.pattern().toUtf8().data());
   if( !       environment.isEmpty()) printf("Environment        = \"%s\"\n",       environment.toUtf8().data());
   if( !           cmd_pre.isEmpty()) printf("Pre Command        = \"%s\"\n",           cmd_pre.toUtf8().data());
   if( !          cmd_post.isEmpty()) printf("Post Command       = \"%s\"\n",          cmd_post.toUtf8().data());
   if( !        customdata.isEmpty()) printf("Custom Data        = \"%s\"\n",        customdata.toUtf8().data());
   if( !   need_properties.isEmpty()) printf("Need Properties    = \"%s\"\n",   need_properties.pattern().toUtf8().data());
   if(     need_power               ) printf("Need Power         =  %u\n"   ,   need_power  );
   if(     need_memory              ) printf("Need Memory        =  %u\n"   ,   need_memory );
   if(     need_hdd                 ) printf("Need HDD           =  %u\n"   ,   need_hdd    );

   if( isNumeric())
      printf("numeric: start = %d, end = %d, perTask = %d, increment = %d.\n", frame_first, frame_last, frame_pertask, frame_inc);
   else if( tasksdata == NULL ) return;
   // Not numeric block not filled with tasks will exit here

   if( full )
   {
      for( int t = 0; t < tasksnum; t++)
      {
         printf("#%d:", t);
         TaskExec * task = genTask(t);
         printf(" N'%s'", task->getName().toUtf8().data());
         printf(" C'%s'", task->getCommand().toUtf8().data());
         if( task->hasFiles()) printf(" F'%s'", task->getFiles().toUtf8().data());
         delete task;
         printf("\n");
      }
   }
   printf("Memory: %d bytes\n", calcWeight());
}


// Functions to update tasks progress and progeress bar:
// (for information purpoces only, no meaning for server)

bool BlockData::updateProgress( JobProgress * progress)
{
   bool changed = false;

   if( updateBars( progress))
      changed = true;

   uint32_t new_state                  = 0;
   int32_t  new_tasksready             = 0;
   int32_t  new_tasksdone              = 0;
   int32_t  new_taskserror             = 0;
   int32_t  new_percentage             = 0;
   bool     new_tasksskipped           = false;
   uint32_t new_taskssumruntime        = 0;

   for( int t = 0; t < tasksnum; t++)
   {
      uint32_t task_state   = progress->tp[blocknum][t]->state;
      int8_t   task_percent = 0;

      if( task_state & AFJOB::STATE_READY_MASK   )
      {
         new_tasksready++;
      }
      if( task_state & AFJOB::STATE_DONE_MASK    )
      {
         new_tasksdone++;
         task_percent = 100;
         new_taskssumruntime += progress->tp[blocknum][t]->time_done - progress->tp[blocknum][t]->time_start;
      }
      if( task_state & AFJOB::STATE_RUNNING_MASK )
      {
         task_percent = progress->tp[blocknum][t]->percent;
         if( task_percent <  0 ) task_percent = 0;
         else
         if( task_percent > 99 ) task_percent = 99;
      }
      if( task_state & AFJOB::STATE_ERROR_MASK   )
      {
         new_taskserror++;
         task_percent = 0;
         new_taskssumruntime += progress->tp[blocknum][t]->time_done - progress->tp[blocknum][t]->time_start;
      }
      if( task_state & AFJOB::STATE_SKIPPED_MASK   )
      {
         new_tasksskipped = true;
         new_taskssumruntime += progress->tp[blocknum][t]->time_done - progress->tp[blocknum][t]->time_start;
      }

      new_percentage += task_percent;
   }
   new_percentage = new_percentage / tasksnum;

   if(( p_tasksready          != new_tasksready             )||
      ( p_tasksdone           != new_tasksdone              )||
      ( p_taskserror          != new_taskserror             )||
      ( p_percentage          != new_percentage             )||
      ( p_taskssumruntime     != new_taskssumruntime        ))
      changed = true;

   p_tasksready         = new_tasksready;
   p_tasksdone          = new_tasksdone;
   p_taskserror         = new_taskserror;
   p_percentage         = new_percentage;
   p_taskssumruntime    = new_taskssumruntime;

   if( new_tasksready)
   {
      new_state = new_state | AFJOB::STATE_READY_MASK;
      new_state = new_state & (~AFJOB::STATE_DONE_MASK);
   }
   else
   {
      new_state = new_state & (~AFJOB::STATE_READY_MASK);
   }

   if( runningtasks_counter )
   {
      new_state = new_state |   AFJOB::STATE_RUNNING_MASK;
      new_state = new_state & (~AFJOB::STATE_DONE_MASK);
   }
   else
   {
      new_state = new_state & (~AFJOB::STATE_RUNNING_MASK);
   }

   if( new_tasksdone == tasksnum ) new_state = new_state |   AFJOB::STATE_DONE_MASK;
   else                            new_state = new_state & (~AFJOB::STATE_DONE_MASK);

   if( new_taskserror) new_state = new_state |   AFJOB::STATE_ERROR_MASK;
   else                new_state = new_state & (~AFJOB::STATE_ERROR_MASK);

   if( new_tasksskipped) new_state = new_state |   AFJOB::STATE_SKIPPED_MASK;
   else                  new_state = new_state & (~AFJOB::STATE_SKIPPED_MASK);

   if( state & AFJOB::STATE_WAITDEP_MASK)
      new_state = new_state & (~AFJOB::STATE_READY_MASK);

   bool depend = state & AFJOB::STATE_WAITDEP_MASK;
   state = new_state;
   if( depend ) state = state | AFJOB::STATE_WAITDEP_MASK;

   return changed;
}

bool BlockData::updateBars( JobProgress * progress)
{
   bool changed = false;

   for( int pb = 0; pb < AFJOB::PROGRESS_BYTES; pb++)
   {
      p_bar_done[pb]    = 0;
      p_bar_running[pb] = 0;
   }

   for( int t = 0; t < tasksnum; t++)
   {
      if( progress->tp[blocknum][t]->state & AFJOB::STATE_DONE_MASK    )
      {
         setProgress( p_bar_done,    t, true  );
         setProgress( p_bar_running, t, false );
      }
   }
   for( int t = 0; t < tasksnum; t++)
   {
      if( progress->tp[blocknum][t]->state & AFJOB::STATE_READY_MASK   )
      {
         setProgress( p_bar_done,    t, false  );
         setProgress( p_bar_running, t, false  );
      }
   }
   for( int t = 0; t < tasksnum; t++)
   {
      if( progress->tp[blocknum][t]->state & AFJOB::STATE_RUNNING_MASK )
      {
         setProgress( p_bar_done,    t, false );
         setProgress( p_bar_running, t, true  );
      }
   }
   for( int t = 0; t < tasksnum; t++)
   {
      if( progress->tp[blocknum][t]->state & AFJOB::STATE_ERROR_MASK   )
      {
         setProgress( p_bar_done,    t, true  );
         setProgress( p_bar_running, t, true  );
      }
   }
   //stdOutFlags();

   return changed;
}

void BlockData::setProgressBit( uint8_t *array, int pos, bool value)
{
   int byte = pos / 8;
   int bit = pos - byte*8;
   uint8_t flag = 1 << bit;
   if( value ) array[byte] = array[byte] |   flag;
   else        array[byte] = array[byte] & (~flag);
//printf(" %d %d %d ", byte, bit, flag);
}

void BlockData::setProgress( uint8_t *array, int task, bool value)
{
   int pos_a = AFJOB::PROGRESS_BYTES * 8 *  task    / tasksnum;
   int pos_b = AFJOB::PROGRESS_BYTES * 8 * (task+1) / tasksnum;
   if( pos_b > pos_a) pos_b--;
   if( pos_b > AFJOB::PROGRESS_BYTES * 8 ) pos_b = AFJOB::PROGRESS_BYTES * 8 - 1;
//printf("BlockData::setProgress: task=%d, pos_a=%d, pos_b=%d, value=%d\n", task, pos_a, pos_b, value);
   for( int pos = pos_a; pos <= pos_b; pos++) setProgressBit( array, pos, value);
//printf("\n");
}

void BlockData::stdOutFlags() const
{
   for( int i = 0; i < AFJOB::PROGRESS_BYTES; i++)
   {
      uint8_t flags = 1;
      for( int b = 0; b < 8; b++)
      {
         if( p_bar_done[i] & flags) printf("1"); else printf("0");
         flags <<= 1;
      }
   }
   printf("\n");
   for( int i = 0; i < AFJOB::PROGRESS_BYTES; i++)
   {
      uint8_t flags = 1;
      for( int b = 0; b < 8; b++)
      {
         if( p_bar_running[i] & flags) printf("1"); else printf("0");
         flags <<= 1;
      }
   }
   printf("\n");
}
