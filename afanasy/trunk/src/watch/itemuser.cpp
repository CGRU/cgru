#include "itemuser.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemUser::HeightUser = 30;
const int ItemUser::HeightAnnotation = 14;

ItemUser::ItemUser( af::User *user):
   ItemNode( (af::Node*)user)
{
   updateValues( user, 0);
}

ItemUser::~ItemUser()
{
}

void ItemUser::updateValues( af::Node *node, int type)
{
   af::User * user = (af::User*)node;

   permanent = user->isPermanent();
   if( numrunningtasks ) setRunning();
   else        setNotRunning();

   priority             = user->getPriority();
   annotation           = user->getAnnontation();
   hostname             = user->getHostName();
   numjobs              = user->getNumJobs();
   numrunningtasks      = user->getNumHosts();
   maxhosts             = user->getMaxHosts();
   hostsmask            = user->getHostsMask();
   hostsmask_exclude    = user->getHostsMaskExclude();
   errors_avoidhost     = user->getErrorsAvoidHost();
   errors_tasksamehost  = user->getErrorsTaskSameHost();
   errors_retries       = user->getErrorsRetries();

   if( numrunningtasks ) setRunning();
   else                  setNotRunning();

   priority_str            = QString::number( priority);
   maxhosts_str            = QString::number( maxhosts);
   errorsAvoidHost_str     = QString::number( errors_avoidhost);
   errorsTaskSameHost_str  = QString::number( errors_tasksamehost);
   errorsRetries_str       = QString::number( errors_retries);

   strErrorHosts = "E-" + errorsAvoidHost_str + "j|" + errorsTaskSameHost_str + "t|" + errorsRetries_str + 'r';

   strLeftTop     = name + '-' + priority_str;
   if( false == permanent ) strLeftTop = QString("(%1)").arg( strLeftTop);
   if( isLocked()) strLeftTop = "(LOCK) " + strLeftTop;

   strLeftBottom  = 'j' + QString::number( numjobs) + '/' + QString::number( user->getNumRunningJobs());

   strHCenterTop.clear();
   if( maxhosts != -1) strHCenterTop  = 'm' + maxhosts_str;
   if( false == hostsmask.isEmpty()       )  strHCenterTop += QString(" H(%1)").arg( hostsmask         );
   if( false == hostsmask_exclude.isEmpty()) strHCenterTop += QString(" E(%1)").arg( hostsmask_exclude );
   strHCenterTop += QString(" E-%1j|%2t|%3r").arg( errorsAvoidHost_str, errorsTaskSameHost_str, errorsRetries_str);

   strRightTop    = hostname;

   strRightBottom = 'n' + QString::number( user->getNeed(), 'g', 5);
   if( user->isSolved()) strRightBottom += "_S";

   tooltip = name + ":";
   tooltip += "\nPriority = " + priority_str;
   tooltip += "\nMaximum hosts = " + maxhosts_str;
   if( maxhosts == -1 ) tooltip += " (no limit)";
   tooltip += "\nHosts mask = \"" + hostsmask + '"';
   if( hostsmask.isEmpty() ) tooltip += " (any host)";
   if( false == hostsmask_exclude.isEmpty()   ) tooltip += "\nExclude hosts mask = \"" + hostsmask_exclude + '"';
   tooltip += "\nLast host = " + hostname;
   tooltip += "\nJob errors avoid host = " + errorsAvoidHost_str;
   tooltip += "\nTask errors avoid host = " + errorsTaskSameHost_str;
   tooltip += "\nRetry task errors = " + errorsRetries_str;
   tooltip += "\nJobs count = " + QString::number( user->getNumJobs());
   tooltip += "\nOnline since " + af::time2Qstr( user->getTimeOnline()   );
   if( permanent) tooltip += "\nRegistered at " + af::time2Qstr( user->getTimeRegister());

   calcHeight();
}

bool ItemUser::calcHeight()
{
   int old_height = height;
   height = HeightUser;
   if( false == annotation.isEmpty()) height += HeightAnnotation;
   return old_height == height;
}

void ItemUser::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   drawBack( painter, option);
   int x = option.rect.x(); int y = option.rect.y();
   int w = option.rect.width(); int h = option.rect.height();

   painter->setPen( clrTextMain( option) );
   if( permanent) painter->setFont( afqt::QEnvironment::f_name);
   else           painter->setFont( afqt::QEnvironment::f_info);
   painter->drawText( option.rect, Qt::AlignLeft    | Qt::AlignTop,     strLeftTop);

   painter->setPen( clrTextInfo( option) );
   painter->setFont( afqt::QEnvironment::f_info);
   painter->drawText( x+5, y, w-5, HeightUser, Qt::AlignLeft    | Qt::AlignBottom,  strLeftBottom  );
   painter->drawText( x+5, y, w-5, HeightUser, Qt::AlignHCenter | Qt::AlignTop,     strHCenterTop  );
   painter->drawText( x+5, y, w-5, HeightUser, Qt::AlignRight   | Qt::AlignBottom,  strRightBottom );
   painter->setPen( afqt::QEnvironment::qclr_black );
   painter->drawText( x+5, y, w-5, HeightUser, Qt::AlignRight   | Qt::AlignTop,     strRightTop    );

   if( false == annotation.isEmpty())
      painter->drawText( option.rect, Qt::AlignBottom | Qt::AlignHCenter, annotation );

   {  // draw stars:
      static const int stars_size = 8;
      static const int stars_height = 21;
      static const int stars_left = 35;
      static const int stars_maxdelta = stars_size * 2 + 5;

      int quantity = numrunningtasks;
      //quantity = 155;

      if( quantity < 1) return;
      int numstars = quantity;
      int stars_right = w - 50;
      int stars_delta = (stars_right - stars_left) / numstars;
      if( stars_delta < 1 )
      {
         stars_delta = 1;
         numstars = stars_right - stars_left;
      }
      else if( stars_delta > stars_maxdelta ) stars_delta = stars_maxdelta;
      int sx = x + stars_left;
      for( int j = 0; j < numstars; j++)
      {
         drawStar( stars_size, sx, y + stars_height, painter);
         sx += stars_delta;
      }

      painter->setFont( afqt::QEnvironment::f_name);
      painter->setPen( afqt::QEnvironment::clr_textstars.c);
      painter->drawText( x, y, w, HeightUser, Qt::AlignHCenter | Qt::AlignBottom, QString::number(numrunningtasks));
   }
}

bool ItemUser::setSortType(   int type )
{
   resetSorting();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TPRIORITY:
         sort_int = &priority;
         break;
      case CtrlSortFilter::TNAME:
         sort_str = &name;
         break;
      case CtrlSortFilter::THOSTNAME:
         sort_str = &hostname;
         break;
      case CtrlSortFilter::TNUMJOBS:
         sort_int = &numjobs;
         break;
      case CtrlSortFilter::TNUMRUNNINGTASKS:
         sort_int = &numrunningtasks;
         break;
      default:
         AFERRAR("ItemUser::setSortType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}

bool ItemUser::setFilterType( int type )
{
   resetFiltering();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TNAME:
         filter_str = &name;
         break;
      case CtrlSortFilter::THOSTNAME:
         filter_str = &hostname;
         break;
      default:
         AFERRAR("ItemUser::setFilterType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}
