#pragma once

#include "../include/afgui.h"

#include "../libafqt/qenvironment.h"
#include "../libafqt/name_afqt.h"

#include <QtCore/QLinkedList>
#include <QtCore/QStringList>

class QApplication;
class QKeyEvent;
class QPixmap;

class ButtonMonitor;
class Dialog;
class ItemJob;
class ListItems;
class MonitorHost;
class Popup;
class Receiver;
class Wnd;
class WndList;

class Watch
{
public:
	Watch( Dialog * pDialog, QApplication * pApplication);
	~Watch();

	static void destroy();

	enum MonType{
		WNONE,

		WJobs,
		WUsers,
		WRenders,
		WMonitors,

		WLAST
	};

	static bool isPadawan()  { return afqt::QEnvironment::level.n == AFGUI::PADAWAN; }
	static bool notPadawan() { return afqt::QEnvironment::level.n != AFGUI::PADAWAN; }
	static bool isJedi( )    { return afqt::QEnvironment::level.n == AFGUI::JEDI;    }
	static bool isSith()     { return afqt::QEnvironment::level.n == AFGUI::SITH;    }
	static bool notSith()    { return afqt::QEnvironment::level.n != AFGUI::SITH;    }

	static const QString BtnName[WLAST];
	static const QString WndName[WLAST];

	static bool isInitialized();
	static bool isConnected();

	static void connectionLost();
	static void connectionEstablished();

	static void sendMsg( af::Msg * msg);


	static void get( const std::string & i_str);
	static void get( const char * i_type,
		const std::vector<int32_t> & i_ids,
		const std::vector<std::string> & i_modes = std::vector<std::string>(),
		const std::vector<int32_t> & i_blocks = std::vector<int32_t>());


	static void addWindow(      Wnd      * wnd      );
	static void removeWindow(   Wnd      * wnd      );
	static void addReceiver(    Receiver * receiver );
	static void removeReceiver( Receiver * receiver );

	static void caseMessage( af::Msg * msg);

	static void filesReceived( const af::MCTaskUp & i_taskup);

	static void setWindowTitle( const QString & title);

	static void displayInfo(    const QString &message);
	static void displayWarning( const QString &message);
	static void displayError(   const QString &message);

	static void showDocs();
	static void showForum();

	static bool openMonitor( int type, bool open);

	static void listenJob(  int id, const QString & name);
	inline static void listenJob_rem(  int id) { ms_listenjobids.removeAll(id);}
	static void watchJodTasksWindowAdd( int id, const QString & name);
	static void watchJodTasksWindowRem( int id);
	static void listenTask( int jobid, int block, int task, const QString & name);

	inline static Dialog * getDialog()  { return ms_d;}
	static void keyPressEvent( QKeyEvent * event);

	static void ntf_JobAdded( const ItemJob * i_job);
	static void ntf_JobDone(  const ItemJob * i_job);
	static void ntf_JobError( const ItemJob * i_job);

	static WndList* opened[WLAST];

	static void raiseWindow( QWidget * wnd, const QString * name = NULL);

	static void repaint();
	static void repaintStart();
	static void repaintFinish();

	static void startProcess( const QString & i_cmd,
			const QString & i_wdir = QString(),
			const std::map<std::string,std::string> & i_env_map = std::map<std::string,std::string>());

	inline static const QPixmap * getServiceIconLarge( const QString & service_name) { return ms_services_icons_large.value( service_name, NULL);}
	inline static const QPixmap * getServiceIconSmall( const QString & service_name) { return ms_services_icons_small.value( service_name, NULL);}

	void static refreshGui();

	void static loadImage( QPixmap & o_pixmap, const QString & i_filename);

	void static browseImages( const QString & i_image,  const QString & i_wdir);
	void static browseFolder( const QString & i_folder, const QString & i_wdir = QString());

	void static notify( const QString & i_title, const QString & i_msg = QString(), uint32_t i_state = 0);

private:
	static Dialog * ms_d;
	static QApplication * ms_app;

	static QLinkedList<Wnd*> ms_windows;
	static QLinkedList<Receiver*> ms_receivers;

	static QLinkedList<int> ms_listenjobids;
	static QLinkedList<int> ms_watchtasksjobids;
	static QLinkedList<QWidget*> ms_watchtaskswindows;

	static QMap<QString, QPixmap *> ms_services_icons_large;
	static QMap<QString, QPixmap *> ms_services_icons_small;
};
