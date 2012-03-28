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

BlockData::BlockData():
   jobid( 0),
   blocknum(0)
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
   state = AFJOB::STATE_READY_MASK;
   flags = 0;
   frame_first = 0;
   frame_last = 0;
   frame_pertask = 1;
   frame_inc = 1;
   maxrunningtasks = -1;
   maxruntasksperhost = -1;
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

   memset( p_bar_running, 0, AFJOB::PROGRESS_BYTES);
   memset( p_bar_done,    0, AFJOB::PROGRESS_BYTES);
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
AFINFA("BlockData::BlockData(): JobId=%d, BlockNum=%d", jobid, blocknum)
   initDefaults();
   construct();
}

void BlockData::construct()
{
   tasksnum = 0;
   tasksdata = NULL;
   runningtasks_counter = 0;

   dependmask.setCaseSensitive();
   tasksdependmask.setCaseSensitive();

   hostsmask.setCaseInsensitive();

   hostsmask_exclude.setCaseInsensitive();
   hostsmask_exclude.setExclude();

   need_properties.setCaseSensitive();
   need_properties.setContain();
}

bool BlockData::isValid() const
{
   if( tasksnum == 0)
   {
      AFERRAR("BlockData::isValid: #%d block[%s] zero tasks number.", blocknum, name.c_str())
      return false;
   }
   if( isNotNumeric())
   {
      if( tasksdata == NULL)
      {
         AFERRAR("BlockData::isValid: #%d block[%s] tasks data is null on not numeric block.", blocknum, name.c_str())
         return false;
      }
      for( int t = 0; t < tasksnum; t++)
      {
         if(tasksdata[t] == NULL)
         {
            AFERRAR("BlockData::isValid: #%d block[%s] task[%d] data is null on not numeric block.", blocknum, name.c_str(), t)
            return false;
         }
      }
   }
   return true;
}

BlockData::~BlockData()
{
//printf("BlockData::~BlockData()\n");
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
      rw_uint32_t( flags,                 msg);
      if( isNotNumeric()) rw_tasks(       msg);

   case Msg::TBlocksProperties:
      rw_int32_t ( parsercoeff,           msg);
      rw_String  ( tasksname,             msg);
      rw_String  ( parser,                msg);
      rw_String  ( wdir,                  msg);
      rw_String  ( environment,           msg);
      rw_String  ( command,               msg);
      rw_String  ( files,                 msg);
      rw_String  ( cmd_pre,               msg);
      rw_String  ( cmd_post,              msg);
      rw_String  ( multihost_service,     msg);
      rw_String  ( customdata,            msg);

   case Msg::TJobsList:
      rw_uint32_t( flags,                 msg);
      rw_int64_t ( frame_first,           msg);
      rw_int64_t ( frame_last,            msg);
      rw_int64_t ( frame_pertask,         msg);
      rw_int64_t ( frame_inc,             msg);
      rw_int64_t ( filesize_min,          msg);
      rw_int64_t ( filesize_max,          msg);
      rw_int32_t ( capcoeff_min,          msg);
      rw_int32_t ( capcoeff_max,          msg);
      rw_uint8_t ( multihost_min,         msg);
      rw_uint8_t ( multihost_max,         msg);
      rw_uint16_t( multihost_waitsrv,     msg);
      rw_uint16_t( multihost_waitmax,     msg);
      rw_int32_t ( capacity,              msg);
      rw_int32_t ( maxrunningtasks,       msg);
      rw_int32_t ( maxruntasksperhost,    msg);
      rw_int32_t ( need_memory,           msg);
      rw_int32_t ( need_power,            msg);
      rw_int32_t ( need_hdd,              msg);
      rw_RegExp  ( dependmask,            msg);
      rw_RegExp  ( tasksdependmask,       msg);
      rw_RegExp  ( hostsmask,             msg);
      rw_RegExp  ( hostsmask_exclude,     msg);
      rw_RegExp  ( need_properties,       msg);
      rw_String  ( name,                  msg);
      rw_String  ( service,               msg);
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
      rw_int64_t ( p_taskssumruntime,     msg);

      rw_uint32_t( state,                 msg);
      rw_int32_t ( jobid,                 msg);
      rw_int32_t ( blocknum,              msg);

   break;

   default:
      AFERRAR("BlockData::readwrite: invalid type = %s.", Msg::TNAMES[msg->type()])
   }
//printf("BlockData::readwrite: END\n");
}

void BlockData::rw_tasks( Msg * msg)
{
   if( isNumeric())
   {
      AFERROR("BlockData::rw_tasks: block is numeric.")
      return;
   }
   rw_int32_t( tasksnum, msg);
   if( tasksnum < 1)
   {
      AFERRAR("BlockData::rw_tasks: invalid number of tasks = %d.", tasksnum)
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
            AFERROR("BlockData::rw_tasks: Can not allocate memory for new task.")
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
   AFERRAR("BlockData::setCapacity: invalid capacity = %d", value)
   return false;
}

bool BlockData::setCapacityCoeffMin( int value)
{
   if( canVarCapacity() == false)
   {
      AFERROR("BlockData::setCapacityCoeffMin: Block can't variate capacity.")
      return false;
   }
   capcoeff_min = value;
   return true;
}

bool BlockData::setCapacityCoeffMax( int value)
{
   if( canVarCapacity() == false)
   {
      AFERROR("BlockData::setCapacityCoeffMax: Block can't variate capacity.")
      return false;
   }
   capcoeff_max = value;
   return true;
}

bool BlockData::setMultiHostMin( int value)
{
   if( isMultiHost() == false)
   {
      AFERROR("BlockData::setMultiHostMin: Block is not multihost.")
      return false;
   }
   if( value < 1)
   {
      AFERROR("BlockData::setMultiHostMin: Hosts minimum can't be less than one.")
      return false;
   }
   multihost_min = value;
   return true;
}

bool BlockData::setMultiHostMax( int value)
{
   if( isMultiHost() == false)
   {
      AFERROR("BlockData::setMultiHostMax: Block is not multihost.")
      return false;
   }
   if( value < multihost_min)
   {
      AFERROR("BlockData::setMultiHostMax: Hosts maximum can't be less than minimum.")
      return false;
   }
   multihost_max = value;
   return true;
}

bool BlockData::setNumeric( long long start, long long end, long long perTask, long long increment)
{
   if( perTask < 1)
   {
      AFERRAR("BlockData::setNumeric(): Frames per task = %lld < 1 ( setting to 1).", perTask)
      perTask = 1;
   }
   if( increment < 1)
   {
      AFERRAR("BlockData::setNumeric(): Frames increment = %lld < 1 ( setting to 1).", increment)
      increment = 1;
   }
   if( tasksdata)
   {
      AFERROR("BlockData::setNumeric(): this block already has tasks.")
      return false;
   }
   if( isNumeric())
   {
      AFERROR("BlockData::setNumeric(): this block is already numeric and numbers are set.")
      return false;
   }
   if( start > end)
   {
      AFERRAR("BlockData::setNumeric(): start > end ( %lld > %lld - setting end to %lld)", start, end, start)
      end = start;
   }
   flags = flags | FNumeric;

   frame_first    = start;
   frame_last     = end;
   frame_pertask  = perTask;
   frame_inc      = increment;

   long long numframes = (frame_last - frame_first) / frame_inc + 1;
   tasksnum = numframes / frame_pertask;
   if((numframes%perTask) != 0) tasksnum++;

   return true;
}

bool BlockData::genNumbers( long long & start, long long & end, int num, long long * frames_num) const
{
   start = 0;
   end = 0;
   if( frames_num ) *frames_num = 1;

   if( num > tasksnum)
   {
      AFERROR("BlockData::genNumbers: n > tasksnum.")
      return false;
   }
   if( frame_pertask == 0)
   {
      AFERROR("BlockData::genNumbers: frame_pertask == 0.")
      return false;
   }

   if( isNotNumeric() )
   {
      if( frame_pertask > 0 )
      {
         start = num * frame_pertask;
         end = start + frame_pertask - 1;
         if( frames_num ) *frames_num = frame_pertask;
      }
      else
      {
         start = num / (-frame_pertask);
         end = start;//( num + 1 ) / (-frame_pertask);
         if( frames_num ) *frames_num = -frame_pertask;
      }
      return true;
   }

   long long offset = num * frame_pertask * frame_inc;
   if( frame_inc > 1 ) offset -= offset % frame_inc;
   start = frame_first + offset;

   offset = frame_pertask * frame_inc - 1;
   if(( start + offset ) > frame_last ) offset = frame_last - start;
   if( frame_inc > 1 ) offset -= offset % frame_inc;
   end = start + offset;

   if( frames_num )
   {
      if( frame_inc > 1 )
         *frames_num = ( end - start ) / frame_inc + 1;
      else
         *frames_num = end - start + 1;
   }

   return true;
}

bool BlockData::calcTaskNumber( long long frame, int & tasknum) const
{
   tasknum = frame - frame_first;

   if( frame_pertask > 0 ) tasknum = tasknum / frame_pertask;
   else tasknum = tasknum * (-frame_pertask);

   if( frame_inc > 1 ) tasknum = tasknum / frame_inc;

   bool retval = true;
   if( tasknum < 0 )
   { 
      tasknum = 0;
      retval = false;
   }
   if( tasknum >= tasksnum )
   {
      tasknum = tasksnum - 1;
      retval = false;
   }
   if(( frame < frame_first ) || ( frame > frame_last )) retval = false;
   return retval;
}

void BlockData::setFramesPerTask( long long perTask)
{
   if( perTask == 0)
   {
      AFERROR("BlockData::setFramesPerTask: Can't set frames per task to zero.")
      return;
   }
   if( isNumeric())
   {
      AFERROR("BlockData::setFramesPerTask: The block is numeric.")
      return;
   }
   frame_pertask = perTask;
}

const std::string BlockData::genCommand( int num, long long * fstart, long long * fend) const
{
   std::string str;
   if( num > tasksnum)
   {
      AFERROR("BlockData::getCmd: n > tasksnum.")
      return str;
   }
   if( isNumeric())
   {
      long long start, end;
      bool ok = true;
      if( fstart && fend )
      {
         start = *fstart;
         end   = *fend;
      }
      else
         ok = genNumbers( start, end, num);

      if( ok)
         str = fillNumbers( command, start, end);
   }
   else
   {
      str = af::replaceArgs( command, tasksdata[num]->getCommand());
   }
   return str;
}

const std::string BlockData::genFiles( int num, long long * fstart, long long * fend) const
{
   std::string str;
   if( num >= tasksnum)
   {
      AFERROR("BlockData::genCmdView: n >= tasksnum.")
      return str;
   }
   if( isNumeric())
   {
      if( files.size())
      {
         long long start, end;
         bool ok = true;
         if( fstart && fend )
         {
            start = *fstart;
            end   = *fend;
         }
         else
            ok = genNumbers( start, end, num);

         if( ok)
            str = af::fillNumbers( files, start, end);
      }
   }
   else
   {
      str = af::replaceArgs( files, tasksdata[num]->getFiles());
   }
   return str;
}

TaskExec * BlockData::genTask( int num) const
{
   if( num > tasksnum)
   {
      AFERROR("BlockData::genTask: n > tasksnum.")
      return NULL;
   }

   long long start = -1;
   long long end = -1;
   long long frames_num = -1;

   if( false == genNumbers( start, end, num, &frames_num)) return NULL;

   return new TaskExec(
         genTaskName( num, &start, &end),
         service,
         parser,
         genCommand( num, &start, &end),
         capacity,
         filesize_min,
         filesize_max,
         genFiles( num, &start, &end),

         start,
         end,
         frames_num,

         wdir,
         environment,

         jobid,
         blocknum,
         num
      );
}

const std::string BlockData::genTaskName( int num, long long * fstart, long long * fend) const
{
   if( num > tasksnum)
   {
      AFERROR("BlockData::genTaskName: n > tasksnum.")
      return std::string ("> tasksnum");
   }

   if( isNumeric())
   {
      long long start, end;
      bool ok = true;
      if( fstart && fend )
      {
         start = *fstart;
         end   = *fend;
      }
      else
         ok = genNumbers( start, end, num);

      if( false == ok) return std::string("Error generating numbers.");

      if( tasksname.size()) return fillNumbers( tasksname, start, end);

      std::string str("frame ");
      str += itos( start);
      if( start != end )
      {
         str += std::string("-") + itos( end);
         if( frame_inc > 1 ) str += std::string(" / ") + itos( frame_inc);
      }
      return str;
   }

   return af::replaceArgs( tasksname, tasksdata[num]->getName());
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
   weight += dependmask.weigh();
   weight += tasksdependmask.weigh();
   weight += need_properties.weigh();
   weight += hostsmask.weigh();
   weight += hostsmask_exclude.weigh();
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

const std::string BlockData::generateInfoStringTyped( int type, bool full) const
{
   std::ostringstream stream;
   generateInfoStreamTyped( stream, type, full);
   return stream.str();
}

void BlockData::generateInfoStreamTyped( std::ostringstream & stream, int type, bool full) const
{
   switch( type)
   {
/*   case Msg::TJob:
   case Msg::TJobRegister:
   case Msg::TBlocks:
      break;*/

   case Msg::TBlocksProperties:

      if( full ) stream << "\nProperties:";

      if( isNumeric())
      {
         stream << "\n Frames: " << frame_first << " - " << frame_last << ": Per Task = " << frame_pertask;
         if( frame_inc > 1 ) stream << ", Increment = " << frame_inc;
      }

      if( full && ( parsercoeff != 1 )) stream << "\n Parser Coefficient = " << parsercoeff;

      if( false == tasksname.empty()) stream << "\n Tasks Name Pattern = " << tasksname;

      if( full || ( ! parser.empty())) stream << "\n Parser = " << parser;
      if( full && (   parser.empty())) stream << " is empty (no parser)";

      if( false == wdir.empty()) stream << "\n Working Directory:\n" << wdir;

      if( false == command.empty()) stream << "\n Command:\n" << command;

      if( false == environment.empty()) stream << "\n Environment = " << environment;

      if( false == files.empty()) stream << "\n Files:\n" << af::strReplace( files, ';', '\n');

      if( false == cmd_pre.empty()) stream << "\n Pre Command:\n" << cmd_pre;
      if( false == cmd_post.empty()) stream << "\n Post Command:\n" << cmd_post;

      if( false == multihost_service.empty()) stream << "\n MultiHost Service = " << multihost_service;

      if( false == customdata.empty()) stream << "\n Custom Data:\n" << customdata;

//      break;
//   case Msg::TJobsList:

      if(( filesize_min != -1 ) && ( filesize_max != -1 )) stream << "Files Size: " << filesize_min << " - " << filesize_max;

      if( full ) stream << "\n Capacity = " << capacity;
      if( canVarCapacity()) stream << " x" << capcoeff_min << " - x" << capcoeff_max;

      if( isMultiHost() )
      {
         stream << "\n MultiHost: x" << multihost_min << " -x" << multihost_max;
         stream << "\n    Wait service start = " << multihost_waitsrv;
         stream << "\n    Wait service start maximum = " << multihost_waitmax;
      }

      if( isDependSubTask() ) stream << "\n   Sub Task Dependence.";
      if( isNonSequential() ) stream << "\n   Non-sequential tasks solving.";

      if( full || ( maxrunningtasks != -1 )) stream << "\n Max Running Tasks = " << maxrunningtasks;
      if( full && ( maxrunningtasks == -1 )) stream << " (no limit)";

      if( need_memory > 0           ) stream << "\n Needed Memory = "   << need_memory;
      if( need_power  > 0           ) stream << "\n Need Power = "      << need_power;
      if( need_hdd    > 0           ) stream << "\n Need HDD = "        << need_hdd;
      if( need_properties.notEmpty()) stream << "\n Need Properties = " << need_properties.getPattern();

      if(        dependmask.notEmpty()) stream << "\n Depend Mask = "         << dependmask.getPattern();
      if(   tasksdependmask.notEmpty()) stream << "\n Tasks Depend Mask = "   << tasksdependmask.getPattern();
      if(         hostsmask.notEmpty()) stream << "\n Hosts Mask = "          << hostsmask.getPattern();
      if( hostsmask_exclude.notEmpty()) stream << "\n Exclude Hosts Mask = "  << hostsmask_exclude.getPattern();

      if( full ) stream << "\n Service = " << service;
      if( full ) stream << "\n Tasks Number = " << tasksnum;

      if( full ) stream << "\nErrors solving:";
      if( full || ( maxrunningtasks     != -1 )) stream << "\n Maximum running tasks = " << maxrunningtasks;
      if( full && ( maxrunningtasks     == -1 )) stream << " (no limit)";
      if( full || ( errors_avoidhost    != -1 )) stream << "\n Errors for block avoid host = " << int(errors_avoidhost);
      if( full && ( errors_avoidhost    == -1 )) stream << " (user settings used)";
      if( full || ( errors_tasksamehost != -1 )) stream << "\n Errors for task avoid host = " << int( errors_tasksamehost);
      if( full && ( errors_tasksamehost == -1 )) stream << " (user settings used)";
      if( full || ( errors_retries      != -1 )) stream << "\n Error task retries = " << int( errors_retries);
      if( full && ( errors_retries      == -1 )) stream << " (user settings used)";
      if( full || ( errors_forgivetime  != -1 )) stream << "\n Errors forgive time = " << errors_forgivetime << " seconds";
      if( full && ( errors_forgivetime  == -1 )) stream << " (infinite)";

      break;

   case Msg::TBlocksProgress:

      if( full ) stream << "\nRunning Progress:";

      if( p_tasksdone ) stream << "\n Run Time: Sum = " << af::time2strHMS( p_taskssumruntime, true)
            << " / Average = " << af::time2strHMS( p_taskssumruntime/p_tasksdone, true);

      if( full ) stream << "\n Tasks Ready = " << p_tasksready;
      if( full ) stream << "\n Tasks Done = " << p_tasksdone;
      if( full ) stream << "\n Tasks Error = " << p_taskserror;

      if( p_errorhostsnum ) stream << "\n Error hosts count = " << p_errorhostsnum;
      if( p_avoidhostsnum ) stream << "\n Avoid hosts count = " << p_avoidhostsnum;

      break;

   default:
      stream << "Can not generate type info for type = " << Msg::TNAMES[type];
      break;
   }
}

void BlockData::generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "Block[" << name << "] " << service << "[" << capacity << "] " << tasksnum << " tasks";
   generateInfoStreamTyped( stream, Msg::TBlocksProgress,   full);
   generateInfoStreamTyped( stream, Msg::TBlocksProperties, full);
//   if( full ) generateInfoStreamTyped( stream, Msg::TBlocks, full);
/*
   if( full )
   {
      if( isNumeric())
      {
         stream << "\n numeric: start = " << frame_first
               << ", end = " << frame_last
               << ", perTask = " << frame_pertask
               << ", increment = " << frame_inc << ".";
      }
      else if( tasksdata != NULL )
      {
         for( int t = 0; t < tasksnum; t++)
         {
            stream << std::endl;
            stream << "#" << t << ":";
            TaskExec * task = genTask(t);
            stream << " [" << task->getName() << "] ";
            stream << "'" << task->getCommand() << "'";
            if( task->hasFiles()) stream << " (" << task->getFiles() << ")";
            delete task;
         }
      }
      stream << " memory: " << calcWeight() << " bytes.";
   }
*/
}


// Functions to update tasks progress and progeress bar:
// (for information purpoces only, no meaning for server)

bool BlockData::updateProgress( JobProgress * progress)
{
   bool changed = false;

   if( updateBars( progress))
      changed = true;

   uint32_t  new_state                  = 0;
   int32_t   new_tasksready             = 0;
   int32_t   new_tasksdone              = 0;
   int32_t   new_taskserror             = 0;
   int32_t   new_percentage             = 0;
   bool      new_tasksskipped           = false;
   long long new_taskssumruntime        = 0;

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

void BlockData::stdOutProgress() const
{
   std::cout << generateProgressString() << std::endl;
}

const std::string BlockData::generateProgressString() const
{
   std::ostringstream stream;
   generateProgressStream( stream);
   return stream.str();
}

void BlockData::generateProgressStream( std::ostringstream & stream) const
{
   for( int i = 0; i < AFJOB::PROGRESS_BYTES; i++)
   {
      uint8_t flags = 1;
      for( int b = 0; b < 8; b++)
      {
         if( p_bar_done[i] & flags) stream << "1"; else stream << "0";
         flags <<= 1;
      }
   }
   stream << std::endl;
   for( int i = 0; i < AFJOB::PROGRESS_BYTES; i++)
   {
      uint8_t flags = 1;
      for( int b = 0; b < 8; b++)
      {
         if( p_bar_running[i] & flags) stream << "1"; else stream << "0";
         flags <<= 1;
      }
   }
   stream << std::endl;
}
