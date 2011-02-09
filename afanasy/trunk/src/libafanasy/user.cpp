#include "user.h"

#include <stdio.h>
#include <memory.h>

#include <QtCore/QDateTime>

#include "msg.h"
#include "address.h"
#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

User::User( const QString &username, const QString &host):
   hostname( host),
   maxrunningtasks(  af::Environment::getMaxRunningTasksNumber() ),
   hostsmask( af::Environment::getHostsMask()),
   errors_retries(    af::Environment::getTaskErrorRetries() ),
   errors_avoidhost( af::Environment::getErrorsAvoidHost()  ),
   errors_tasksamehost( af::Environment::getTaskErrorsSameHost() ),
   errors_forgivetime( af::Environment::getErrorsForgiveTime()),
   time_register( 0)
{
   name = username;
   priority = af::Environment::getPriority();

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

   hostsmask.setCaseSensitivity( Qt::CaseInsensitive);
   hostsmask_exclude.setCaseSensitivity( Qt::CaseInsensitive);
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
   rw_QString ( hostname,              msg);
   rw_int32_t ( maxrunningtasks,       msg);
   rw_uint8_t ( errors_retries,        msg);
   rw_uint8_t ( errors_avoidhost,      msg);
   rw_uint8_t ( errors_tasksamehost,   msg);
   rw_int32_t ( errors_forgivetime,    msg);
   rw_uint32_t( time_online,           msg);
   rw_int32_t ( numjobs,               msg);
   rw_int32_t ( numrunningjobs,        msg);
   rw_int32_t ( runningtasksnumber,    msg);
   rw_float   ( need,                  msg);
   rw_QRegExp ( hostsmask,             msg);
   rw_QRegExp ( hostsmask_exclude,     msg);
   rw_QString ( annotation,            msg);
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
   weight += weigh(hostsmask);
   weight += weigh(hostsmask_exclude);
   weight += weigh(annotation);
   return weight;
}

void User::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
      stream << "User id=" << id << ", name = \"" << name.toUtf8().data() << "\":";
      stream << "\n\tJobs = " << numjobs << " / active jobs = " << numrunningjobs;
      stream << "\n\tMaximum Running Tasks Number = " << maxrunningtasks;
      stream << "\n\tRunning Tasks Number = " << runningtasksnumber;
      if( hasHostsMask())        stream << "\n\tHosts Mask = \"" << hostsmask.pattern().toUtf8().data() << "\"";
      if( hasHostsMaskExclude()) stream << "\n\tExclude Hosts Mask = \"" << hostsmask_exclude.pattern().toUtf8().data() << "\"\n";
      stream << "\n\tRegistration time = " << time2str( time_register);
      stream << "\n\tOnline time = " << time2str( time_online);
      stream << "\n\tHost = " << hostname.toUtf8().data();
      stream << "\n\tPriority = " << int(priority);
      if( isPermanent()) stream << "\n\tstatus = PERMANENT"; else stream << "\n\tstatus = temporary";
      stream << "\n\tWeight = " << calcWeight() << " bytes.";
   }
   else
   {
      stream << "#" << id << ":" << int(priority)
            << " " << name.toUtf8().data()
            << " j" << numjobs << "/" << numrunningjobs
            << " r" << runningtasksnumber << "/" << maxrunningtasks
            << " " << time2str( time_online)
            << " " <<  hostname.toUtf8().data()
            << " " << (isPermanent() == 1 ? "P" : "T")
            << " - " << calcWeight() << " bytes.";
   }
}
