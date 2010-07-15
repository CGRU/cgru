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
   maxhosts(  af::Environment::getMaxHosts() ),
   hostsmask( af::Environment::getHostsMask()),
   errors_retries(    af::Environment::getTaskErrorRetries() ),
   errors_avoidhost( af::Environment::getErrorsAvoidHost()  ),
   errors_tasksamehost( af::Environment::getTaskErrorsSameHost() ),
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
   numhosts = 0;
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
   rw_int32_t ( maxhosts,              msg);
   rw_uint8_t ( errors_retries,        msg);
   rw_uint8_t ( errors_avoidhost,      msg);
   rw_uint8_t ( errors_tasksamehost,   msg);
   rw_uint32_t( time_online,           msg);
   rw_int32_t ( numjobs,               msg);
   rw_int32_t ( numrunningjobs,        msg);
   rw_int32_t ( numhosts,              msg);
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

void User::stdOut( bool full) const
{
   if( full)
   {
      printf("User id=%d:\n", id);
      printf("Name = %s:\n", name.toUtf8().data());
      printf("Jobs = %d / active jobs = %d\n", numjobs, numrunningjobs);
      printf("Max Hosts = \"%d\"\n", maxhosts);
      printf("Num Hosts = \"%d\"\n", numhosts);
      if( hasHostsMask()) printf("Hosts Mask = \"%s\"\n", hostsmask.pattern().toUtf8().data());
      if( hasHostsMaskExclude()) printf("Exclude Hosts Mask = \"%s\"\n", hostsmask_exclude.pattern().toUtf8().data());
      printf("Registration time = %s\n", time2Qstr( time_register).toUtf8().data());
      printf("Online time = %s\n", time2Qstr( time_online).toUtf8().data());
      printf("Host = %s\n", hostname.toUtf8().data());
      printf("Priority = %d\n", priority);
      if( isPermanent()) printf("status = PERMANENT\n"); else printf("status = temporary\n");
      printf("Weight = %d bytes.\n", calcWeight());
   }
   else
   {
      printf("#%d:%d %s j%d/%d h%d/%d \"%s\"/\"%s\" %s %s %s - %d bytes.\n",
         id,
         priority,
         name.toUtf8().data(),
         numjobs,
         numrunningjobs,
         maxhosts,
         numhosts,
         hostsmask.pattern().toUtf8().data(),
         hostsmask_exclude.pattern().toUtf8().data(),
         time2Qstr( time_online).toUtf8().data(),
         hostname.toUtf8().data(),
         (isPermanent() == 1 ? "P" : "T"),
         calcWeight()
      );
   }
}
