#include "listusers.h"

#include <QtGui/QMenu>
#include <QtCore/QEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QContextMenuEvent>

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "../libafqt/qmsg.h"

#include "itemuser.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

int     ListUsers::SortType       = CtrlSortFilter::TNUMJOBS;
bool    ListUsers::SortAscending  = false;
int     ListUsers::FilterType     = CtrlSortFilter::TNAME;
bool    ListUsers::FilterInclude  = true;
bool    ListUsers::FilterMatch    = false;
QString ListUsers::FilterString   = "";

ListUsers::ListUsers( QWidget* parent):
   ListNodes(  parent, af::Msg::TUsersListRequest)
{
   ctrl = new CtrlSortFilter( this, &SortType, &SortAscending, &FilterType, &FilterInclude, &FilterMatch, &FilterString);
   ctrl->addSortType(   CtrlSortFilter::TNONE);
   ctrl->addSortType(   CtrlSortFilter::TPRIORITY);
   ctrl->addSortType(   CtrlSortFilter::TNAME);
   ctrl->addSortType(   CtrlSortFilter::THOSTNAME);
   ctrl->addSortType(   CtrlSortFilter::TNUMJOBS);
   ctrl->addSortType(   CtrlSortFilter::TNUMRUNNINGTASKS);
   ctrl->addFilterType( CtrlSortFilter::TNONE);
   ctrl->addFilterType( CtrlSortFilter::TNAME);
   ctrl->addFilterType( CtrlSortFilter::THOSTNAME);
   initSortFilterCtrl();

   eventsShowHide << af::Msg::TMonitorUsersAdd;
   eventsShowHide << af::Msg::TMonitorUsersChanged;
   eventsOnOff    << af::Msg::TMonitorUsersDel;

   parentWindow->setWindowTitle("Users");

   init();

   if( false == af::Environment::VISOR()) setAllowSelection( false);

   connect( (ModelNodes*)model, SIGNAL( nodeAdded( ItemNode *, const QModelIndex &)),
                          this,   SLOT( userAdded( ItemNode *, const QModelIndex &)));
}

ListUsers::~ListUsers()
{
}

void ListUsers::contextMenuEvent(QContextMenuEvent *event)
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   bool me = false;
   if( useritem->getId() == Watch::getUid()) me = true;


   QMenu menu(this);
   QAction *action;

   action = new QAction( "Show Log", this);
   connect( action, SIGNAL( triggered() ), this, SLOT( actRequestLog() ));
   menu.addAction( action);

   if( me || af::Environment::VISOR() )
   {
      menu.addSeparator();

      action = new QAction( "Annonate", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actAnnotate() ));
      menu.addAction( action);

      menu.addSeparator();

      action = new QAction( "Set Max Running Tasks", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actMaxRunningTasks() ));
      menu.addAction( action);
      action = new QAction( "Set Hosts Mask", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actHostsMask() ));
      menu.addAction( action);
      action = new QAction( "Set Exclude Hosts Mask", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actHostsMaskExclude() ));
      menu.addAction( action);
      action = new QAction( "Set Prority", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actPriority() ));
      menu.addAction( action);

      menu.addSeparator();

      action = new QAction( "Set Job Avoid Errors", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actErrorsAvoidHost() ));
      menu.addAction( action);
      action = new QAction( "Set Task Avoid Errors", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actErrorsSameHost() ));
      menu.addAction( action);
      action = new QAction( "Set Task Error Retries", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actErrorRetries() ));
      menu.addAction( action);
      action = new QAction( "Set Errors Forgive Time", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actErrorsForgiveTime() ));
      menu.addAction( action);

      menu.addSeparator();

      action = new QAction( "Set Jobs Life Time", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actJobsLifeTime() ));
      menu.addAction( action);
   }

   if( af::Environment::VISOR())
   {
      menu.addSeparator();
      action = new QAction( "Add", this);
      action->setEnabled( false == useritem->isPermanent());
      connect( action, SIGNAL( triggered() ), this, SLOT( actAdd() ));
      menu.addAction( action);
   }
   if( af::Environment::GOD())
   {
      menu.addSeparator();
      action = new QAction( "Delete", this);
      action->setEnabled( useritem->isPermanent());
      connect( action, SIGNAL( triggered() ), this, SLOT( actDelete() ));
      menu.addAction( action);
   }

   menu.exec( event->globalPos());
}

bool ListUsers::caseMessage( af::Msg * msg)
{
AFINFO("ListUsers::caseMessage( Msg msg)\n");
#ifdef AFOUTPUT
   msg->stdOut();
#endif
   switch( msg->type())
   {
   case af::Msg::TUsersList:
   {
      updateItems( msg);
      calcTitle();
      subscribe();
      break;
   }
   case af::Msg::TMonitorUsersDel:
   {
      af::MCGeneral ids( msg);
      deleteItems( ids);
      calcTitle();
      break;
   }
   case af::Msg::TMonitorUsersAdd:
   {
      af::MCGeneral ids( msg);
      deleteItems( ids);
      Watch::sendMsg( new afqt::QMsg( af::Msg::TUsersListRequestIds, &ids, true));
      break;
   }
   case af::Msg::TMonitorUsersChanged:
   {
      af::MCGeneral ids( msg);
      Watch::sendMsg( new afqt::QMsg( af::Msg::TUsersListRequestIds, &ids, true));
      break;
   }
   default:
      return false;
   }
   return true;
}

ItemNode* ListUsers::createNewItem( af::Node *node)
{
   return new ItemUser( (af::User*)node);
}

void ListUsers::userAdded( ItemNode * node, const QModelIndex & index)
{
//printf("node->getId()=%d ,   Watch::getUid()=%d,  row=%d\n", node->getId(), Watch::getUid(), index.row());
   if( node->getId() == Watch::getUid()) view->selectionModel()->select( index, QItemSelectionModel::Select);
}

void ListUsers::calcTitle()
{
   int total = count();
   int running = 0;
   for( int i = 0; i < total; i++)
   {
      ItemUser * itemuser = (ItemUser*)(model->item(i));
      if( itemuser->numrunningtasks > 0 ) running++;
   }
   parentWindow->setWindowTitle(QString("U[%1]: %2R").arg( total).arg( running));
}

void ListUsers::actAnnotate()
{
   ItemUser* item = (ItemUser*)getCurrentItem();
   if( item == NULL ) return;
   QString current = item->annotation;

   bool ok;
   QString text = QInputDialog::getText(this, "Annotate", "Enter Annotation", QLineEdit::Normal, current, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( text);
   action( mcgeneral, af::Msg::TUserAnnotate);
}

void ListUsers::actPriority()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   int current = useritem->priority;

   int maximum = af::Environment::getPriority();
   if( af::Environment::VISOR()) maximum = 250;
   bool ok;
   uint8_t priority = QInputDialog::getInteger(this, "Change Priority", "Enter New Priority", current, 0, maximum, 1, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( priority);
   action( mcgeneral, af::Msg::TUserPriority);
}

void ListUsers::actErrorsAvoidHost()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   int current = useritem->errors_avoidhost;

   bool ok;
   uint8_t value = QInputDialog::getInteger(this, "Errors to avoid host", "Enter Number of Errors", current, 0, 99, 1, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( value);
   action( mcgeneral, af::Msg::TUserErrorsAvoidHost);
}

void ListUsers::actErrorsSameHost()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   int current = useritem->errors_tasksamehost;

   bool ok;
   uint8_t value = QInputDialog::getInteger(this, "Errors same host", "Enter Number of Errors", current, 0, 99, 1, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( value);
   action( mcgeneral, af::Msg::TUserErrorsTaskSameHost);
}

void ListUsers::actErrorRetries()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   int current = useritem->errors_retries;

   bool ok;
   uint8_t value = QInputDialog::getInteger(this, "Auto retry error tasks", "Enter Number of Errors", current, 0, 99, 1, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( value);
   action( mcgeneral, af::Msg::TUserErrorRetries);
}

void ListUsers::actErrorsForgiveTime()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   double cur = double( useritem->errors_forgivetime ) / (60.0*60.0);

   bool ok;
   double hours = QInputDialog::getDouble( this, "Errors Forgive Time", "Enter number of hours (0=infinite)", cur, 0, 365*24, 3, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( int( hours * 60.0 * 60.0 ));
   action( mcgeneral, af::Msg::TUserErrorsForgiveTime);
}

void ListUsers::actJobsLifeTime()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   double cur = double( useritem->jobs_lifetime ) / (60.0*60.0);

   bool ok;
   double hours = QInputDialog::getDouble( this, "Jobs Life Time", "Enter number of hours (0=infinite)", cur, 0, 365*24, 3, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( int( hours * 60.0 * 60.0 ));
   action( mcgeneral, af::Msg::TUserJobsLifeTime);
}

void ListUsers::actMaxRunningTasks()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   int current = useritem->maxrunningtasks;

   bool ok;
   int max = QInputDialog::getInteger(this, "Change Maximum Running Tasks", "Enter Number", current, -1, 9999, 1, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( max);
   action( mcgeneral, af::Msg::TUserMaxRunningTasks);
}

void ListUsers::actHostsMask()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   QString current = useritem->hostsmask;

   bool ok;
   QString mask = QInputDialog::getText(this, "Change Hosts Mask", "Enter Mask", QLineEdit::Normal, current, &ok);
   if( !ok) return;

   QRegExp rx( mask, Qt::CaseInsensitive);
   if( rx.isValid() == false )
   {
      displayError( rx.errorString());
      return;
   }

   af::MCGeneral mcgeneral( mask);
   action( mcgeneral, af::Msg::TUserHostsMask);
}

void ListUsers::actHostsMaskExclude()
{
   ItemUser* useritem = (ItemUser*)getCurrentItem();
   if( useritem == NULL ) return;
   QString current = useritem->hostsmask_exclude;

   bool ok;
   QString mask = QInputDialog::getText(this, "Change Exclude Mask", "Enter Mask", QLineEdit::Normal, current, &ok);
   if( !ok) return;

   QRegExp rx( mask, Qt::CaseInsensitive);
   if( rx.isValid() == false )
   {
      displayError( rx.errorString());
      return;
   }

   af::MCGeneral mcgeneral( mask);
   action( mcgeneral, af::Msg::TUserHostsMaskExclude);
}

void ListUsers::actAdd()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TUserAdd);
}

void ListUsers::actDelete()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TUserDel);
}

void ListUsers::actRequestLog()
{
   displayInfo( "User log request.");
   Item* item = getCurrentItem();
   if( item == NULL ) return;
   afqt::QMsg * msg = new afqt::QMsg( af::Msg::TUserLogRequestId, item->getId(), true);
   Watch::sendMsg( msg);
}
