#include "listrenders.h"

#include <QtCore/QEvent>
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
bool    ListRenders::FilterMatch    = false;
QString ListRenders::FilterString   = "";

ListRenders::ListRenders( QWidget* parent):
   ListNodes( parent, af::Msg::TRendersListRequest)
{
   ctrl = new CtrlSortFilter( this, &SortType, &SortAscending, &FilterType, &FilterInclude, &FilterMatch, &FilterString);
   ctrl->addSortType(   CtrlSortFilter::TNONE);
   ctrl->addSortType(   CtrlSortFilter::TPRIORITY);
   ctrl->addSortType(   CtrlSortFilter::TCAPACITY);
   ctrl->addSortType(   CtrlSortFilter::TTIMELAUNCHED);
   ctrl->addSortType(   CtrlSortFilter::TTIMEREGISTERED);
   ctrl->addSortType(   CtrlSortFilter::TNAME);
   ctrl->addSortType(   CtrlSortFilter::TTASKUSER);
   ctrl->addSortType(   CtrlSortFilter::TUSERNAME);
   ctrl->addSortType(   CtrlSortFilter::TVERSION);
   ctrl->addFilterType( CtrlSortFilter::TNONE);
   ctrl->addFilterType( CtrlSortFilter::TNAME);
   ctrl->addFilterType( CtrlSortFilter::TUSERNAME);
   ctrl->addFilterType( CtrlSortFilter::TTASKUSER);
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
      if( getSelectedItemsCount() == 1) action->setEnabled(false == render->isNIMBY());
      connect( action, SIGNAL( triggered() ), this, SLOT( actNIMBY() ));
      menu.addAction( action);
      action = new QAction( "Set nimby", this);
      if( getSelectedItemsCount() == 1) action->setEnabled(false == render->isnimby());
      connect( action, SIGNAL( triggered() ), this, SLOT( actNimby() ));
      menu.addAction( action);
      action = new QAction( "Set Free", this);
      if( getSelectedItemsCount() == 1) action->setEnabled(render->isnimby() || render->isNIMBY());
      connect( action, SIGNAL( triggered() ), this, SLOT( actFree() ));
      menu.addAction( action);
      action = new QAction( "Set User", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actUser() ));
      menu.addAction( action);

      menu.addSeparator();

      action = new QAction( "Annotate", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actAnnotate() ));
      menu.addAction( action);
      action = new QAction( "Change Capacity", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actCapacity() ));
      menu.addAction( action);
      action = new QAction( "Enable Service", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actEnableService() ));
      menu.addAction( action);
      action = new QAction( "Disable Service", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actDisableService() ));
      menu.addAction( action);
      action = new QAction( "Restore Defaults", this);
      if( getSelectedItemsCount() == 1) action->setEnabled(render->isDirty());
      connect( action, SIGNAL( triggered() ), this, SLOT( actRestoreDefaults() ));
      menu.addAction( action);

      menu.addSeparator();

      action = new QAction( "Eject Tasks", this);
      if( getSelectedItemsCount() == 1) action->setEnabled(render->hasTasks());
      connect( action, SIGNAL( triggered() ), this, SLOT( actEject() ));
      menu.addAction( action);
   }

   if( af::Environment::GOD())
   {
      menu.addSeparator();

      action = new QAction( "Exit Render", this);
      if( getSelectedItemsCount() == 1) action->setEnabled(render->isOnline());
      connect( action, SIGNAL( triggered() ), this, SLOT( actExit() ));
      menu.addAction( action);
      action = new QAction( "Delete Render", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actDelete() ));
      if( getSelectedItemsCount() == 1) action->setEnabled(false == render->isOnline());
      menu.addAction( action);
      action = new QAction( "Restart Render", this);
      if( getSelectedItemsCount() == 1) action->setEnabled(render->isOnline());
      connect( action, SIGNAL( triggered() ), this, SLOT( actRestart() ));
      menu.addAction( action);
      action = new QAction( "Reboot Computer", this);
      if( getSelectedItemsCount() == 1) action->setEnabled(render->isOnline());
      connect( action, SIGNAL( triggered() ), this, SLOT( actReboot() ));
      menu.addAction( action);

/* Do not needed since multitask renders, but can be used in future.
      action = new QAction( "Start another Render", this);
      connect( action, SIGNAL( triggered() ), this, SLOT( actStart() ));
      menu.addAction( action);
*/
   }

   int cmdssize = af::Environment::getRenderCmds().size();
   if( cmdssize > 0 )
   {
      menu.addSeparator();

      QMenu * submenu = new QMenu( "Custom", this);
      for( int i = 0; i < cmdssize; i++)
      {
         ActionId * actionid = new ActionId( i, QString("%1").arg( af::Environment::getRenderCmds()[i]), this);
         connect( actionid, SIGNAL( triggeredId( int ) ), this, SLOT( actCommand( int ) ));
         submenu->addAction( actionid);
      }
      menu.addMenu( submenu);
   }

   menu.addSeparator();

   action = new QAction( "Shutdown Computer", this);
   if( getSelectedItemsCount() == 1) action->setEnabled(render->isOnline());
   connect( action, SIGNAL( triggered() ), this, SLOT( actShutdown() ));
   menu.addAction( action);

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
void ListRenders::actCapacity()
{
   ItemRender* item = (ItemRender*)getCurrentItem();
   if( item == NULL ) return;
   int current = item->getCapacity();

   bool ok;
   int32_t capacity = QInputDialog::getInteger(this, "Change Capacity", "Enter New Capacity", current, 0, 1000000, 1, &ok);
   if( !ok) return;
   af::MCGeneral mcgeneral( capacity);
   action( mcgeneral, af::Msg::TRenderCapacity);
}
void ListRenders::actNIMBY()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderNIMBY);
}
void ListRenders::actNimby()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderNimby);
}
void ListRenders::actFree()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderFree);
}
void ListRenders::actUser()
{
   QString current = af::Environment::getUserName();

   bool ok;
   QString text = QInputDialog::getText(this, "Set User", "Enter User Name", QLineEdit::Normal, current, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( text);
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

void ListRenders::actAnnotate()
{
   ItemRender* item = (ItemRender*)getCurrentItem();
   if( item == NULL ) return;
   QString current = item->getAnnotation();

   bool ok;
   QString text = QInputDialog::getText(this, "Annotate", "Enter Annotation", QLineEdit::Normal, current, &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral( text);
   action( mcgeneral, af::Msg::TRenderAnnotate);
}

void ListRenders::actEnableService()  { setService( true );}
void ListRenders::actDisableService() { setService( false);}
void ListRenders::setService( bool enable)
{
   Item* item = getCurrentItem();
   if( item == NULL ) return;
   QString caption("Service");
   if( enable ) caption = "Enable " + caption; else caption = "Disable " + caption;

   bool ok;
   QString service = QInputDialog::getText(this, caption, "Enter Service Name", QLineEdit::Normal, QString(), &ok);
   if( !ok) return;

   af::MCGeneral mcgeneral;
   mcgeneral.setString( service);
   mcgeneral.setNumber( enable);
   action( mcgeneral, af::Msg::TRenderSetService);
}

void ListRenders::actRestoreDefaults()
{
   af::MCGeneral mcgeneral;
   action( mcgeneral, af::Msg::TRenderRestoreDefaults);
}

void ListRenders::actCommand( int number)
{
   if( number >= af::Environment::getRenderCmds().size())
   {
      displayError( "No such command.");
      return;
   }

   QModelIndexList indexes( view->selectionModel()->selectedIndexes());

   QString cmd(af::Environment::getRenderCmds()[number]);
   if( cmd.contains( AFWATCH::CMDS_ASKCOMMAND))
   {
      bool ok;
      QString text = QInputDialog::getText(this, "Launch Command",
         QString("Entrer string to replace %1 in\n%2").arg(AFWATCH::CMDS_ASKCOMMAND).arg(cmd), QLineEdit::Normal, "", &ok);
      if( !ok) return;
      cmd = cmd.replace( AFWATCH::CMDS_ASKCOMMAND, text);
   }

   if( indexes.count() < 1 )
   {
      Item* item = getCurrentItem();
      Watch::startProcess( cmd.replace( AFWATCH::CMDS_ARGUMENT, item->getName()));
      return;
   }

   for( int i = 0; i < indexes.count(); i++)
   {
      if( false == qVariantCanConvert<Item*>( indexes[i].data())) continue;
      Item* item = qVariantValue<Item*>( indexes[i].data());
      if( item == NULL ) continue;
      QString final_command(cmd);
      Watch::startProcess( final_command.replace( AFWATCH::CMDS_ARGUMENT, item->getName()));
   }
}
