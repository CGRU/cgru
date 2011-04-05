#include "user.h"

#include <stdio.h>
#include <memory.h>

//#include <QtCore/QDateTime>

#include "msg.h"
#include "address.h"
#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

User::User( const std::string & username, const std::string & host):
   hostname( host),
   maxrunningtasks(  af::Environment::getMaxRunningTasksNumber() ),
   errors_retries(    af::Environment::getTaskErrorRetries() ),
   errors_avoidhost( af::Environment::getErrorsAvoidHost()  ),
   errors_tasksamehost( af::Environment::getTaskErrorsSameHost() ),
   errors_forgivetime( af::Environment::getErrorsForgiveTime()),
   jobs_lifetime( 0),
   time_register( 0)
{
   name = username;
   priority = af::Environment::getPriority();
   hostsmask.setPattern( af::Environment::getHostsMask());

   construct();
}

User::User( int uid)
{
   id = uid;
   construct();
}

User::User( Msg * msg)
{
   read( msg);
}

void User::construct()
{
   state = 0;
   numjobs = 0;
   numrunningjobs = 0;
   need = 0.0f;
   runningtasksnumber = 0;
   time_online = 0;

   hostsmask.setCaseInsensitive();

   hostsmask_exclude.setCaseInsensitive();
   hostsmask_exclude.setExclude();
}

User::~User()
{
AFINFO("User::~User:\n");
}

void User::readwrite( Msg * msg)
{
   Node::readwrite( msg);

   rw_uint32_t( state,                 msg);
   rw_uint32_t( time_register,         msg);
   rw_String  ( hostname,              msg);
   rw_int32_t ( maxrunningtasks,       msg);
   rw_uint8_t ( errors_retries,        msg);
   rw_uint8_t ( errors_avoidhost,      msg);
   rw_uint8_t ( errors_tasksamehost,   msg);
   rw_int32_t ( errors_forgivetime,    msg);
   rw_uint32_t( time_online,           msg);
   rw_int32_t ( jobs_lifetime,         msg);
   rw_int32_t ( numjobs,               msg);
   rw_int32_t ( numrunningjobs,        msg);
   rw_int32_t ( runningtasksnumber,    msg);
   rw_float   ( need,                  msg);
   rw_RegExp  ( hostsmask,             msg);
   rw_RegExp  ( hostsmask_exclude,     msg);
   rw_String  ( annotation,            msg);
//   rw_String  ( customdata,            msg);
}

void User::setPermanent( bool value)
{
   if( value )
   {
      state = state | Permanent;
      time_register = time( NULL);
   }
   else
      state = state & (~Permanent);
}

int User::calcWeight() const
{
   int weight = Node::calcWeight();
   weight += sizeof(User) - sizeof( Node);
   weight += weigh(hostname);
   weight += hostsmask.weigh();
   weight += hostsmask_exclude.weigh();
   weight += weigh(annotation);
   return weight;
}

const std::string User::generateErrorsSolvingString() const
{
   std::ostringstream stream;
   generateErrorsSolvingStream( stream);
   return stream.str();
}

void User::generateErrorsSolvingStream( std::ostringstream & stream) const
{
   stream << "E-" << int(errors_avoidhost) << "j|"
         << int(errors_tasksamehost) << "t|"
         << int(errors_retries) << "r";
   if( errors_forgivetime > 0 ) stream << " F" << af::time2strHMS( errors_forgivetime, true);
}

void User::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
      stream << "User name = \"" << name << "\" (id=" << id << "):";
      stream << "\n Priority = " << int(priority);
      stream << "\n    Each point gives 10% bonus";
      stream << "\n Jobs = " << numjobs << " / active jobs = " << numrunningjobs;
      if( jobs_lifetime > 0 ) stream << "\n Jobs life time = " << af::time2strHMS( jobs_lifetime, true);
      stream << "\n Maximum Running Tasks = " << maxrunningtasks;
      if( maxrunningtasks < 1 ) stream << " (no limit)";
      stream << "\n Running Tasks Number = " << runningtasksnumber;
      if( hasHostsMask())        stream << "\n Hosts Mask = \"" << hostsmask.getPattern() << "\"";
      if( hasHostsMaskExclude()) stream << "\n Exclude Hosts Mask = \"" << hostsmask_exclude.getPattern() << "\"";

      stream << "\n Errors Solving: ";
      generateErrorsSolvingStream( stream);
      stream << "\n    Errors To Avoid Host = " << int(errors_avoidhost);
      stream << "\n    Maximum Errors Same Task = " << int(errors_tasksamehost);
      stream << "\n    Task Errors To Retry = " << int(errors_retries);
      stream << "\n    Errors Forgive Time = " << af::time2strHMS( errors_forgivetime, true);
      if( errors_forgivetime == 0 ) stream << " (infinite, no forgiving)";

      if( hostname.size() != 0) stream << "\n Last host = \"" << hostname << "\"";
      if( isPermanent())
      {
         stream << "\n User is permanent (stored in database)";
         stream << "\n Registration time = " << time2str( time_register);
      }
      else stream << "\n (user is temporal)";
      stream << "\n Online time = " << time2str( time_online);
      stream << "\n Need hosts order = " << need;
      if( isSolved()) stream << " (solved)";
      stream << "\n    need = pow(1.1, Priority) / (RunTasks + 1)";
      if( annotation.size()) stream << "\n" << annotation;
      if( customdata.size()) stream << "\nCustom Data:\n" << customdata;
      //stream << "\n Memory = " << calcWeight() << " bytes.";
   }
   else
   {
      stream << "#" << id << ":" << int(priority)
            << " " << name
            << " j" << numjobs << "/" << numrunningjobs
            << " r" << runningtasksnumber << "/" << maxrunningtasks
            << " " << time2str( time_online)
            << " " <<  hostname
            << " " << (isPermanent() == 1 ? "P" : "T")
            << " - " << calcWeight() << " bytes.";
   }
}
