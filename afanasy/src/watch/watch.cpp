#include "watch.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "blockinfo.h"
#include "buttonmonitor.h"
#include "dialog.h"
#include "item.h"
#include "monitorhost.h"
#include "listtasks.h"
#include "reciever.h"
#include "wndlist.h"
#include "wndlistenjob.h"
#include "wndlistentask.h"

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtGui/QApplication>
#include <QtGui/QPixmap>
#include <QtGui/QSound>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const QString Watch::BtnName[WLAST] = { "NULL","JOBS","USERS","RENDERS","T","M"};
const QString Watch::WndName[WLAST] = { "null","Jobs","Users","Renders","Talks","Monitors"};
WndList* Watch::opened[WLAST] = {0,0,0,0,0,0};

QLinkedList<Wnd*>      Watch::ms_windows;
QLinkedList<Reciever*> Watch::ms_recievers;
QLinkedList<int>       Watch::ms_listenjobids;
QLinkedList<int>       Watch::ms_watchtasksjobids;
QLinkedList<QWidget*>  Watch::ms_watchtaskswindows;

QStringList Watch::ms_previewcmds;
QStringList Watch::ms_rendercmds;

QMap<QString, QPixmap *> Watch::ms_services_icons_large;
QMap<QString, QPixmap *> Watch::ms_services_icons_small;

QApplication * Watch::ms_app = NULL;
Dialog * Watch::ms_d = NULL;
MonitorHost * Watch::ms_m = NULL;

Watch::Watch( Dialog * pDialog, QApplication * pApplication)
{
   ms_app = pApplication;
   ms_d = pDialog;
   ms_m = ms_d->getMonitor();

// Get services icons:
   QDir dir( afqt::stoq( af::Environment::getCGRULocation()) + "/icons/software");
   if( false == dir.exists()) return;
   QFileInfoList files = dir.entryInfoList();
   if( files.size() == 0) return;
   for( int i = 0; i < files.size(); i++)
   {
      if( false == files[i].isFile()) continue;
      if( ms_services_icons_large.contains( files[i].completeBaseName()) || ms_services_icons_small.contains( files[i].completeBaseName())) continue;
      QPixmap icon( files[i].filePath());
      if( icon.isNull())
      {
         AFERRAR("Invalid service icon:\n%s", files[i].filePath().toUtf8().data())
         continue;
      }
      ms_services_icons_large[ files[i].completeBaseName()] = new QPixmap( icon.scaledToHeight( BlockInfo::Height,        Qt::SmoothTransformation));
      ms_services_icons_small[ files[i].completeBaseName()] = new QPixmap( icon.scaledToHeight( BlockInfo::HeightCompact, Qt::SmoothTransformation));
   }
}

Watch::~Watch()
{
// Delete services icons:
   for( QMap<QString, QPixmap *>::iterator it = ms_services_icons_large.begin(); it != ms_services_icons_large.end(); it++) delete *it;
   for( QMap<QString, QPixmap *>::iterator it = ms_services_icons_small.begin(); it != ms_services_icons_small.end(); it++) delete *it;
}

void Watch::destroy() { ms_d = NULL; ms_m = NULL;}

void Watch::sendMsg( af::Msg * msg)
{
	if( msg->type() == af::Msg::TJSON )
	{
		std::string str( msg->data(), msg->dataLen());
		printf("\n%s\n", str.c_str());
	}
	if( ms_d ) ms_d->sendMsg( msg);
}

void Watch::displayInfo(    const QString &message){if(ms_d){ms_d->displayInfo(    message);if(ms_d->isHidden())ms_d->show();}}
void Watch::displayWarning( const QString &message){if(ms_d){ms_d->displayWarning( message);if(ms_d->isHidden())ms_d->show();}}
void Watch::displayError(   const QString &message){if(ms_d){ms_d->displayError(   message);if(ms_d->isHidden())ms_d->show();}}
void Watch::setWindowTitle( const QString & title ){if(ms_d){ms_d->setWindowTitle( title  );if(ms_d->isHidden())ms_d->show();}}
void Watch::keyPressEvent( QKeyEvent * event) { if(ms_d) ms_d->keyPressEvent( event);}

bool Watch::isInitialized() { if(ms_d) return ms_d->isInitialized(); else return false;  }
bool Watch::isConnected()   { if(ms_d) return ms_d->isConnected();   else return false;  }
int  Watch::getUid()        { if(ms_d) return ms_d->getUid();        else return 0;      }
int  Watch::getId()         { if(ms_m) return ms_m->getId();         else return 0;      }

const af::Address & Watch::getClientAddress() { return ms_m->getAddress();}

void Watch::subscribe(   const QList<int> & events) { if(ms_m) ms_m->  subscribe( events );}
void Watch::unsubscribe( const QList<int> & events) { if(ms_m) ms_m->unsubscribe( events );}
void Watch::addJobId( int jId ) { if(ms_m) ms_m->addJobId( jId );}
void Watch::delJobId( int jId ) { if(ms_m) ms_m->delJobId( jId );}
void Watch::setUid(   int uid ) { if(ms_m) ms_m->setUid(   uid );}

void Watch::addWindow( Wnd * wnd)
{
   if( ms_windows.contains( wnd))
   {
      AFERROR("Watch::addWindow: Window already exists.")
   }
   else ms_windows.append( wnd);
}
void Watch::addReciever( Reciever * reciever)
{
   if( ms_recievers.contains( reciever))
   {
      AFERROR("Watch::addReciever: Reciever already exists.")
   }
   else ms_recievers.append( reciever);
}
void Watch::removeWindow(   Wnd      * wnd      ) {   ms_windows.removeAll( wnd);      }
void Watch::removeReciever( Reciever * reciever ) { ms_recievers.removeAll( reciever); }

void Watch::caseMessage( af::Msg * msg)
{
   bool recieved = false;
   QLinkedList<Reciever*>::iterator rIt;
   for( rIt = ms_recievers.begin(); rIt != ms_recievers.end(); ++rIt)
   {
      msg->resetWrittenSize();
      if( (*rIt)->caseMessage( msg) && (false == recieved)) recieved = true;
   }
   if( false == recieved)
   {
      AFERROR("Watch::caseMessage: Unknown message recieved:")
      msg->v_stdOut();
   }
}

void Watch::listenJob( int id, const QString & name)
{
   if( ms_listenjobids.contains( id))
      displayWarning("This job is already listening.");
   else
   {
      ms_listenjobids.append( id);
      new WndListenJob( id, name);
   }
}

void Watch::listenTask( int jobid, int block, int task, const QString & name)
{
   new WndListenTask( jobid, block, task, name);
}

void Watch::watchJodTasksWindowAdd( int id, const QString & name)
{
AFINFA("Watch::watchTasks: trying to open job \"%s\"[%d] tasks window.", name.toUtf8().data(), id)
   QLinkedList<int>::const_iterator iIt = ms_watchtasksjobids.begin();
   QLinkedList<QWidget*>::iterator wIt = ms_watchtaskswindows.begin();
   while( iIt != ms_watchtasksjobids.end())
   {
      if( *iIt == id)
      {
         raiseWindow(*wIt, &name);
         return;
      }
      iIt++;
      wIt++;
   }

   WndList * wnd = new WndList("Tasks");
   ListTasks *listtasks = new ListTasks( wnd, id, name);
   wnd->setList( listtasks);
   ms_watchtasksjobids.append( id);
   ms_watchtaskswindows.append( wnd);
   displayInfo(QString("Opening '%1' tasks window.").arg(name));
AFINFA("Watch::watchTasks: \"%s\" window opened.", name.toUtf8().data())
}

void Watch::watchJodTasksWindowRem( int id)
{
   QLinkedList<int>::iterator iIt = ms_watchtasksjobids.begin();
   QLinkedList<QWidget*>::iterator wIt = ms_watchtaskswindows.begin();
   while( iIt != ms_watchtasksjobids.end())
   {
      if( *iIt == id)
      {
         iIt = ms_watchtasksjobids.erase( iIt);
         wIt = ms_watchtaskswindows.erase( wIt);
      }
      else
      {
         iIt++;
         wIt++;
      }
   }
}

void Watch::connectionLost()
{
   for( QLinkedList<Reciever*>::iterator rIt = ms_recievers.begin(); rIt != ms_recievers.end(); ++rIt)
      (*rIt)->v_connectionLost();
   if(ms_m) ms_m->connectionLost();
}

void Watch::connectionEstablished()
{
   for( QLinkedList<Reciever*>::iterator rIt = ms_recievers.begin(); rIt != ms_recievers.end(); ++rIt)
      (*rIt)->v_connectionEstablished();
   if(ms_m) ms_m->connectionEstablished();
}

bool Watch::openMonitor( int type, bool open)
{
   if( ms_d == NULL ) return false;
   if( opened[type])
   {
      raiseWindow( opened[type], &WndName[type]);
      return false;
   }
   return ms_d->openMonitor( type, open);
}

void Watch::raiseWindow( QWidget * wnd, const QString * name)
{
AFINFO("Watch::raiseWindow: trying to raise a window.")
   if((wnd == NULL) || (name == NULL)) return;
   if( wnd->isMinimized()) wnd->showNormal();
   wnd->activateWindow();
   wnd->raise();
   if( name ) displayInfo(QString("Raising '%1' window.").arg(*name));
AFINFA("Watch::raiseWindow: \"%s\" window raised.", name->toUtf8().data())
}

void Watch::startProcess( const QString & i_cmd, const QString & i_wdir)
{
	printf("Starting '%s'", i_cmd.toUtf8().data());
	if( false == i_wdir.isEmpty()) printf(" in '%s'", i_wdir.toUtf8().data());
	printf("\n");

#ifdef WINNT
	PROCESS_INFORMATION pinfo;

	af::launchProgram( &pinfo, i_cmd.toStdString(), i_wdir.toStdString(), NULL, NULL, NULL,
	    CREATE_NEW_CONSOLE, true);

	CloseHandle( pinfo.hThread);
	CloseHandle( pinfo.hProcess);
#else
	af::launchProgram( i_cmd.toStdString(), i_wdir.toStdString(), NULL, NULL, NULL);
#endif
}

void Watch::someJobAdded()
{
    displayInfo("Job added.");
    if( false == afqt::QEnvironment::soundJobAdded.str.isEmpty())
        QSound::play( afqt::QEnvironment::soundJobAdded.str );
}

void Watch::someJobDone()
{
    displayInfo("Job Done.");
    if( false == afqt::QEnvironment::soundJobDone.str.isEmpty())
        QSound::play( afqt::QEnvironment::soundJobDone.str );
}

void Watch::someJobError()
{
    displayWarning("Job Error.");
    if( false == afqt::QEnvironment::soundJobError.str.isEmpty())
        QSound::play( afqt::QEnvironment::soundJobError.str );
}

void Watch::repaintStart()  { if( ms_d) ms_d->repaintStart(100); }
void Watch::repaintFinish() { if( ms_d) ms_d->repaintFinish(); refreshGui(); }

void Watch::refreshGui()
{
    // Calculate star points:
    Item::calcutaleStarPoints();

    // Refresh Images:
    ButtonMonitor::refreshImages();
    if( ms_d) ms_d->reloadImages();

    // Repaint:
    repaint();
}

void Watch::loadImage( QPixmap & o_pixmap, const QString & i_filename)
{
    // Set Pixmap to empty if needed:
    if( i_filename.isEmpty())
    {
        if( false == o_pixmap.isNull())
        {
            o_pixmap = QPixmap();
        }
    }
    else
    {
        // Try to load a file:
        if( false == o_pixmap.load( i_filename) )
        {
            // Try to load a file in current theme folder:
            if( false == o_pixmap.load( afqt::stoq(af::Environment::getAfRoot())
                                + "/icons/watch/"
                                + afqt::QEnvironment::theme.str + "/"
                                + i_filename))
            {
                // Load fails:
                if( false == o_pixmap.isNull())
                {
                    // Set Pixmap to empty if it was not
                    o_pixmap = QPixmap();
                }
            }
        }
    }
}

void Watch::browseImages( const QString & i_image, const QString & i_wdir)
{
    if( i_image.isEmpty())
        return;

    QString folder = i_image.left( i_image.lastIndexOf('/'));
    folder = folder.left( i_image.lastIndexOf('\\'));
    if( folder == i_image )
        folder = i_wdir;

    QDir dir( i_wdir);
    if( dir.exists())
        dir.cd( folder);
    else
        dir.setPath( folder);

    if( false == dir.exists())
    {
        Watch::displayError( QString("Folder '%1' does not exist.").arg( dir.path()));
        return;
    }

	Watch::displayInfo( QString("Opening '%1'").arg( dir.path().toUtf8().data()));
#ifdef WINNT
	QString cmd = "explorer";
#else
	QString cmd = afqt::stoq( af::Environment::getCGRULocation()) + "/utilities/browse.sh";
#endif
	cmd += " \"" + folder + "\"";
	Watch::startProcess( cmd, i_wdir);
}

void Watch::repaint()
{
//printf("Watch::repaint: start\n");
    QPalette palette = ms_app->palette();
    afqt::QEnvironment::setPalette( palette);
    afqt::QEnvironment::initFonts();
    ms_app->setPalette( palette);

    if( ms_d) ms_d->repaint();
    for( int i = 0; i < WLAST; i++) if( opened[i]) opened[i]->repaintItems();
    for( QLinkedList<Wnd*>::iterator wIt = ms_windows.begin(); wIt != ms_windows.end(); wIt++) (*wIt)->update();
//printf("Watch::repaint: finish\n");
}
