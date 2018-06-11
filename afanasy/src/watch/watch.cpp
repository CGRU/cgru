#include "watch.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "../libafqt/qenvironment.h"

#include "blockinfo.h"
#include "buttonmonitor.h"
#include "dialog.h"
#include "item.h"
#include "itemjob.h"
#include "monitorhost.h"
#include "listtasks.h"
#include "popup.h"
#include "receiver.h"
#include "wndlist.h"
#include "wndlistenjob.h"
#include "wndlistentask.h"
#include "wndtask.h"

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtGui/QPixmap>
#include <QApplication>
#include <QSound>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const QString Watch::BtnName[WLAST] = { "null","Jobs","Users","Renders","Monitors"};
const QString Watch::WndName[WLAST] = { "null","Jobs","Users","Renders","Monitors"};
WndList* Watch::opened[WLAST] = {0,0,0,0,0};

QLinkedList<Wnd*>      Watch::ms_windows;
QLinkedList<Receiver*> Watch::ms_receivers;
QLinkedList<int>       Watch::ms_listenjobids;
QLinkedList<int>       Watch::ms_watchtasksjobids;
QLinkedList<QWidget*>  Watch::ms_watchtaskswindows;

QMap<QString, QPixmap *> Watch::ms_services_icons_large;
QMap<QString, QPixmap *> Watch::ms_services_icons_small;

QApplication * Watch::ms_app = NULL;
Dialog * Watch::ms_d = NULL;

Watch::Watch( Dialog * pDialog, QApplication * pApplication)
{
   ms_app = pApplication;
   ms_d = pDialog;

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

void Watch::destroy() { ms_d = NULL; }

void Watch::sendMsg( af::Msg * msg)
{
	if( msg->type() == af::Msg::TJSON )
	{
		msg->setJSONBIN();

		static int unused;
		unused = ::write( 1, " <<< ", 5);
		msg->stdOutData( false);
		unused = ::write( 1, "\n", 1);
	}
	if( ms_d ) ms_d->sendMsg( msg);
}

void Watch::get( const std::string & i_str)
{
	std::string str = "{\"get\":{\"binary\":true,";
	str += i_str + "}}";

	af::Msg * msg = af::jsonMsg( str);
	Watch::sendMsg( msg);
}

void Watch::get(
		const char * i_type,
		const std::vector<int32_t> & i_ids,
		const std::vector<std::string> & i_modes,
		const std::vector<int32_t> & i_blocks)
{
//	{"get":{"type":"jobs","ids":[6],"mode":["progress"],"block_ids":[0]}}
	std::ostringstream str;

	str << "\"type\":\"" << i_type << "\"";

	if( i_ids.size())
	{
		str << ",\"ids\":[";
		for( int i = 0; i < i_ids.size(); i++ ) { if(i) str << ','; str << i_ids[i]; }
		str << "]";
	}

	if( i_modes.size())
	{
		str << ",\"mode\":[";
		for( int i = 0; i < i_modes.size(); i++ ) { if(i) str << ','; str << '"' << i_modes[i] << '"'; }
		str << "]";
	}

	if( i_blocks.size())
	{
		str << ",\"block_ids\":[";
		for( int i = 0; i < i_blocks.size(); i++ ) { if(i) str << ','; str << i_blocks[i]; }
		str << "]";
	}

	get( str.str());
}

void Watch::displayInfo(    const QString &message){if(ms_d){ms_d->displayInfo(    message);if(ms_d->isHidden())ms_d->show();}}
void Watch::displayWarning( const QString &message){if(ms_d){ms_d->displayWarning( message);if(ms_d->isHidden())ms_d->show();}}
void Watch::displayError(   const QString &message){if(ms_d){ms_d->displayError(   message);if(ms_d->isHidden())ms_d->show();}}
void Watch::setWindowTitle( const QString & title ){if(ms_d){ms_d->setWindowTitle( title  );if(ms_d->isHidden())ms_d->show();}}
void Watch::keyPressEvent( QKeyEvent * event) { if(ms_d) ms_d->keyPressEvent( event);}

bool Watch::isInitialized() { if(ms_d) return ms_d->isInitialized(); else return false;  }
bool Watch::isConnected()   { if(ms_d) return ms_d->isConnected();   else return false;  }

void Watch::addWindow( Wnd * wnd)
{
   if( ms_windows.contains( wnd))
   {
      AFERROR("Watch::addWindow: Window already exists.")
   }
   else ms_windows.append( wnd);
}
void Watch::addReceiver( Receiver * receiver)
{
   if( ms_receivers.contains( receiver))
   {
      AFERROR("Watch::addReciever: Receiver already exists.")
   }
   else ms_receivers.append( receiver);
}
void Watch::removeWindow(   Wnd      * wnd      ) {   ms_windows.removeAll( wnd);      }
void Watch::removeReceiver( Receiver * receiver ) { ms_receivers.removeAll( receiver); }

void Watch::caseMessage( af::Msg * msg)
{
   bool received = false;

	QLinkedList<Receiver*>::iterator rIt;
	for( rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
	{
		msg->resetWrittenSize();
		if( (*rIt)->v_caseMessage( msg) && (false == received)) received = true;
	}

	if( msg->type() == af::Msg::TMonitorEvents )
	{
		msg->resetWrittenSize();
		af::MonitorEvents me( msg);
		me.v_stdOut();

		// General instructions for an application:
		if( me.m_instruction.size())
		{
			if( me.m_instruction == "exit")
			{
				printf("Received \"exit\" instrucion. Closing dialog.\n");
				ms_d->close();
				return;
			}
		}

		// Let all receivers to process events:
		for( rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
		{
			msg->resetWrittenSize();
			if( (*rIt)->v_processEvents( me) && (false == received)) received = true;
		}

		for( int i = 0; i < me.m_outputs.size(); i++)
		{
			if( WndTask::showTask( me.m_outputs[i]))
				received = true;
		}

		for( int i = 0; i < me.m_listens.size(); i++)
		{
			if( WndTask::showTask( me.m_listens[i]))
				received = true;
		}

		if( me.m_message.size())
		{
			if( LabelVersion::getStringStatus( me.m_message) != LabelVersion::SS_None )
				ms_d->announce( me.m_message);
			else
				new WndText("Message", me.m_message);

			received = true;
		}
	}
	else if( msg->type() == af::Msg::TTask )
	{
		af::MCTask mctask( msg);
		received = WndTask::showTask( mctask);
	}

   if( false == received)
   {
		printf("Unknown message received: ");
		msg->v_stdOut();
   }
}

void Watch::filesReceived( const af::MCTaskUp & i_taskup)
{
	for( QLinkedList<Receiver*>::iterator rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
	{
		if((*rIt)->v_filesReceived( i_taskup))
			return;
	}

	printf("Watch::filesReceived: Recipient not found:\n");
	i_taskup.v_stdOut();
	return;
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
   for( QLinkedList<Receiver*>::iterator rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
      (*rIt)->v_connectionLost();
}

void Watch::connectionEstablished()
{
   for( QLinkedList<Receiver*>::iterator rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
      (*rIt)->v_connectionEstablished();
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

void Watch::startProcess( const QString & i_cmd, const QString & i_wdir, const std::map<std::string,std::string> & i_env_map)
{
	printf("Starting '%s'", i_cmd.toUtf8().data());
	if( false == i_wdir.isEmpty()) printf(" in '%s'", i_wdir.toUtf8().data());
	printf("\n");

#ifdef WINNT
	PROCESS_INFORMATION pinfo;

	char * env = af::processEnviron( i_env_map);
	af::launchProgram( &pinfo, i_cmd.toStdString(), i_wdir.toStdString(), env,
		NULL, NULL, NULL,
		CREATE_NEW_CONSOLE, true);

	CloseHandle( pinfo.hThread);
	CloseHandle( pinfo.hProcess);
#else
	char ** env = af::processEnviron( i_env_map);
	af::launchProgram( i_cmd.toStdString(), i_wdir.toStdString(), env);
#endif

	if( env )
		delete [] env;
}

void Watch::ntf_JobAdded( const ItemJob * i_job)
{
	displayInfo("Job added.");
	if( false == afqt::QEnvironment::ntf_job_added_sound.str.isEmpty())
		QSound::play( afqt::QEnvironment::ntf_job_added_sound.str );

	if( afqt::QEnvironment::ntf_job_added_alert.n )
		Watch::notify("Job Added", i_job->getName(), i_job->state);
}

void Watch::ntf_JobDone( const ItemJob * i_job)
{
	displayInfo("Job Done.");

	if( false == afqt::QEnvironment::ntf_job_done_sound.str.isEmpty())
		QSound::play( afqt::QEnvironment::ntf_job_done_sound.str );

	if( afqt::QEnvironment::ntf_job_added_alert.n )
		Watch::notify("Job Done", i_job->getName(), i_job->state);
}

void Watch::ntf_JobError( const ItemJob * i_job)
{
	displayWarning("Job Error.");
	if( false == afqt::QEnvironment::ntf_job_error_sound.str.isEmpty())
		QSound::play( afqt::QEnvironment::ntf_job_error_sound.str );

	if( afqt::QEnvironment::ntf_job_added_alert.n )
		Watch::notify("Job Error", i_job->getName(), i_job->state);
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

	Watch::browseFolder( folder, i_wdir);
}

void Watch::browseFolder( const QString & i_folder, const QString & i_wdir)
{
    QDir dir( i_wdir);
    if( dir.exists())
        dir.cd( i_folder);
    else
        dir.setPath( i_folder);

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
	cmd += " \"" + i_folder + "\"";
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

void Watch::notify( const QString & i_title, const QString & i_msg, uint32_t i_state)
{
	new Popup( i_title, i_msg, i_state);
}

void Watch::showDocs() { Watch::startProcess("documentation \"afanasy/gui#watch\""); }
void Watch::showForum() { Watch::startProcess("forum \"watch\""); }

