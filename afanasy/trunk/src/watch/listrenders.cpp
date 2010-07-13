#include "listrenders.h"

#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMenu>

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/address.h"

#include "../libafqt/qmsg.h"

#include "actionid.h"
#include "dialog.h"
#include "itemrender.h"
#include "ctrlrenders.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

bool    ListRenders::ConstHeight    = false;
int     ListRenders::SortType       = CtrlSortFilter::TPRIORITY;
bool    ListRenders::SortAscending  = false;
int     ListRenders::FilterType     = CtrlSortFilter::TNAME;
bool    ListRenders::FilterInclude  = true;
QString ListRenders::FilterString   = "";

ListRenders::ListRenders( QWidget* parent):
   ListNodes( parent, af::Msg::TRendersListRequest)
{
   ctrl = new CtrlSortFilter( this, &SortType, &SortAscending, &FilterType, &FilterInclude, &FilterString);
   ctrl->addSortType(   CtrlSortFilter::TNONE);
   ctrl->addSortType(   CtrlSortFilter::TPRIORITY);
   ctrl->addSortType(   CtrlSortFilter::TNAME);
   ctrl->addSortType(   CtrlSortFilter::TUSERNAME);
   ctrl->addFilterType( CtrlSortFilter::TNONE);
   ctrl->addFilterType( CtrlSortFilter::TNAME);
   ctrl->addFilterType( CtrlSortFilter::TUSERNAME);
   initSortFilterCtrl();

   CtrlRenders * control = new CtrlRenders( ctrl, this);
   control->setToolTip("\
Sort & Filter Renders.\n\
Press RMB for Options.\
");
   ctrl->getLayout()->addWidget( control);

   eventsShowHide << af::Msg::TMonitorRendersAdd;
   eventsShowHide << af::Msg::TMonitorRendersChanged;
   eventsOnOff    << af::Msg::TMonitorRendersDel;

   timer = new QTimer( this);
   connect(timer, SIGNAL(timeout()), this, SLOT( requestResources()));
   timer->start( 990 * af::Environment::getRenderUpdateSec());

   parentWindow->setWindowTitle("Renders");

   init();

   connect( (ModelNodes*)model, SIGNAL(   nodeAdded( ItemNode *, const QModelIndex &)),
                          this,   SLOT( renderAdded( ItemNode *, const QModelIndex &)));

   if( false == af::Environment::VISOR())
      connect( view->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection &)),
                                 this,   SLOT( selectionChanged( const QItemSelection &, const QItemSelection &)));
}

ListRenders::~ListRenders()
{
AFINFO("ListRenders::~ListRenders.\n");
}

void ListRenders::renderAdded( ItemNode * node, const QModelIndex & index)
{
   ItemRender * render = (ItemRender*)node;
   if( af::Environment::VISOR() == false)
   {
      if( render->getName().contains( af::Environment::getComputerName()) || render->getUserName() == af::Environment::getUserName() )
         view->selectionModel()->select( index, QItemSelectionModel::Select);
   }
}

void ListRenders::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
   QModelIndexList indexes = selected.indexes();
   for( int i = 0; i < indexes.count(); i++)
      if( qVariantCanConvert<Item*>( indexes[i].data()))
      {
         ItemRender * render = (ItemRender*)qVariantValue<Item*>( indexes[i].data());
         if((false == render->getName().contains( af::Environment::getComputerName())) && ( render->getUserName() != af::Environment::getUserName()))
            view->selectionModel()->select( indexes[i], QItemSelectionModel::Deselect);
      }
}

void ListRenders::requestResources()
{
   af::MCGeneral ids;
   for( int i = 0; i < model->count(); i++)
   {
      ItemRender *render = (ItemRender*)(model->item(i));
      if( render == NULL) continue;
      if( false == render->isOnline()) continue;
      ids.addId( render->getId());
   }

   if( ids.getCount())
   {
      Watch::sendMsg( new afqt::QMsg( af::Msg::TRendersUpdateRequestIds, &ids, true));
   }
}

void ListRenders::contextMenuEvent( QContextMenuEvent *event)
{
   ItemRender* render = (ItemRender*)getCurrentItem();
   if( render == NULL ) return;
   bool me = false;
   if( render->getName().contains( af::Environment::getComputerName()) || ( render->getUserName() == af::Environment::getUserName())) me = true;

   QMenu menu(this);
   QAction *action;

   action = new QAction( "Show Log", this);
   connect( action, SIGNAL( triggered() ), this, SLOT( actRequestLog() ));
   menu.addAction( action);

   action = new QAction( "Show Services", this);
   connect( action, SIGNAL( triggered() ), this, SLOT( actRequestServices() ));
   menu.addAction( action);

   if( me || af::Environment::VISOR())
   {
      menu.addSeparator();

      action = new QAction( "Set Priority", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actPriority() ));
      menu.addAction( action);
      action = new QAction( "Set NIMBY", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actNIMBY() ));
      menu.addAction( action);
      action = new QAction( "Set nimby", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actNimby() ));
      menu.addAction( action);
      action = new QAction( "Set Free", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actFree() ));
      menu.addAction( action);
      action = new QAction( "Set User", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actUser() ));
      menu.addAction( action);

      menu.addSeparator();

      action = new QAction( "Eject Tasks", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actEject() ));
      menu.addAction( action);
   }

   if( af::Environment::GOD())
   {
      menu.addSeparator();
      action = new QAction( "Exit Render", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actExit() ));
      menu.addAction( action);
      action = new QAction( "Delete Render", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actDelete() ));
      menu.addAction( action);
      action = new QAction( "Restart Render", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actRestart() ));
      menu.addAction( action);
      action = new QAction( "Start another Render", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actStart() ));
      menu.addAction( action);

      menu.addSeparator();

      action = new QAction( "Reboot Computer", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actReboot() ));
      menu.addAction( action);
      action = new QAction( "Shutdown Computer", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actShutdown() ));
      menu.addAction( action);
   }

   const QStringList * cmds = Watch::getRenderCmds();
   int cmdssize = cmds->size();
   if( cmdssize > 0 )
   {
      menu.addSeparator();

      QMenu * submenu = new QMenu( "Custom", this);
      for( int i = 0; i < cmdssize; i++)
      {
         ActionId * actionid = new ActionId( i, QString("%1").arg((*cmds)[i]), this);
         connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actCommand( int ) ));
         submenu->addAction( actionid);
      }
      menu.addMenu( submenu);
   }

   menu.exec( event->globalPos());
}

bool ListRenders::caseMessage( af::Msg * msg)
{
//AFINFO("void ListRenders::caseMessage( Msg msg)\n");
#ifdef AFOUTPUT
//   msg->stdOut();
#endif
   switch( msg->type())
   {
   case af::Msg::TRendersList:
      subscribe();
   case af::Msg::TRendersListUpdates:
   {
      updateItems( msg);
      calcTitle();
      break;
   }
   case af::Msg::TMonitorRendersDel:
   {
      af::MCGeneral ids( msg);
      deleteItems( ids);
      calcTitle();
      break;
   }
   case af::Msg::TMonitorRendersAdd:
   {
      af::MCGeneral ids( msg);
      deleteItems( ids);
      Watch::sendMsg( new afqt::QMsg( af::Msg::TRendersListRequestIds, &ids, true));
      break;
   }
   case af::Msg::TMonitorRendersChanged:
   {
      af::MCGeneral ids( msg);
      Watch::sendMsg( new afqt::QMsg( af::Msg::TRendersListRequestIds, &ids, true));
      break;
   }
   default:
      return false;
   }
   return true;
}

ItemNode* ListRenders::createNewItem( af::Node *node)
{
   return new ItemRender( (af::Render*)node);
}

void ListRenders::calcTitle()
{
   int total = count();
   int online = 0;
   int busy = 0;
   int nimby = 0;
   int free = 0;
   for( int i = 0; i < total; i++)
   {
      ItemRender * itemrender = (ItemRender*)(model->item(i));
      if( itemrender->isOnline()) online++;
      if( itemrender->isBusy()) busy++;
      if( itemrender->isnimby() || itemrender->isNIMBY()) nimby++;
      else if( itemrender->isOnline() && (false == itemrender->isBusy())) free++;
   }
   parentWindow->setWindowTitle(QString("R[%1/%2]: B%3/%4F (n%5)").arg( total).arg( online).arg( busy).arg( free).arg( nimby));
}

void ListRenders::actPriority()
{
   ItemRender* item = (ItemRender*)getCurrentItem();
   if( item == NULL ) return;
   int current = item->getPriority();

   bool ok;
   uint8_t priority = QInputDialog::getInteger(this, "Change Priority", "Enter New Priority", current, 0, 250, 1, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( priority);
   action( mcgeneral, af::Msg::TRenderPriority);
}
void ListRenders::actNIMBY()
{
   af::MCGeneral mcgeneral( af::Environment::getUserName());
   action( mcgeneral, af::Msg::TRenderNIMBY);
}
void ListRenders::actNimby()
{
   af::MCGeneral mcgeneral( af::Environment::getUserName());
   action( mcgeneral, af::Msg::TRenderNimby);
}
void ListRenders::actFree()
{
   af::MCGeneral mcgeneral( af::Environment::getUserName());
   action( mcgeneral, af::Msg::TRenderFree);
}
void ListRenders::actUser()
{
   af::MCGeneral mcgeneral( af::Environment::getUserName());
   action( mcgeneral, af::Msg::TRenderUser);
}
void ListRenders::actEject()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderEject);
}
void ListRenders::actExit()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderExit);
}
void ListRenders::actDelete()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderDelete);
}
void ListRenders::actRestart()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderRestart);
}
void ListRenders::actStart()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderStart);
}
void ListRenders::actReboot()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderReboot);
}
void ListRenders::actShutdown()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderShutdown);
}

void ListRenders::actRequestLog()
{
   Item* item = getCurrentItem();
   if( item == NULL ) return;
   displayInfo( "Render log request.");
   afqt::QMsg * msg = new afqt::QMsg( af::Msg::TRenderLogRequestId, item->getId(), true);
   Watch::sendMsg( msg);
}

void ListRenders::actRequestServices()
{
   Item* item = getCurrentItem();
   if( item == NULL ) return;
   displayInfo( "Render services request.");
   afqt::QMsg * msg = new afqt::QMsg( af::Msg::TRenderServicesRequestId, item->getId(), true);
   Watch::sendMsg( msg);
}

void ListRenders::actCommand( int number)
{
   Item* item = getCurrentItem();
   if( item == NULL )
   {
      displayError( "No items selected.");
      return;
   }

   const QStringList * cmds = Watch::getRenderCmds();
   int cmdssize = cmds->size();
   if( number >= cmdssize )
   {
      displayError( "No such command.");
      return;
   }
   QString cmd((*cmds)[number]);
   cmd = cmd.replace( AFWATCH::CMDS_ARGUMENT, item->getName());
printf("Starting '%s'\n", cmd.toUtf8().data());

   QProcess * process = new QProcess( Watch::getDialog());

   QStringList args;
#ifdef WINNT
   args << "/c" << cmd;
   process->start( "cmd.exe", args, QIODevice::ReadWrite);
#else
   args << "-c" << cmd;
   process->start( "/bin/bash", args, QIODevice::ReadWrite);
#endif
}
