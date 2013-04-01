#pragma once

#include "../libafqt/name_afqt.h"

#include <QtCore/QLinkedList>
#include <QtCore/QStringList>

class QApplication;
class QKeyEvent;
class QPixmap;

class ButtonMonitor;
class Dialog;
class ListItems;
class MonitorHost;
class Reciever;
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
               WTalks,
               WMonitors,

               WLAST
            };

    static const QString BtnName[WLAST];
    static const QString WndName[WLAST];

    static bool isInitialized();
    static bool isConnected();
    static int  getUid();
    static int  getId();

    static void connectionLost();
    static void connectionEstablished();

    static void sendMsg(  af::Msg * msg);

    static void addWindow(      Wnd      * wnd      );
    static void removeWindow(   Wnd      * wnd      );
    static void addReciever(    Reciever * reciever );
    static void removeReciever( Reciever * reciever );

    static void caseMessage( af::Msg * msg);

    static void setWindowTitle( const QString & title);

    static void displayInfo(    const QString &message);
    static void displayWarning( const QString &message);
    static void displayError(   const QString &message);

    static bool openMonitor( int type, bool open);

    static void listenJob(  int id, const QString & name);
    inline static void listenJob_rem(  int id) { ms_listenjobids.removeAll(id);}
    static void watchJodTasksWindowAdd( int id, const QString & name);
    static void watchJodTasksWindowRem( int id);
    static void listenTask( int jobid, int block, int task, const QString & name);

    inline static Dialog * getDialog()  { return ms_d;}
    static const af::Address & getClientAddress();
    static void keyPressEvent( QKeyEvent * event);

    static void   subscribe( const QList<int> & events);
    static void unsubscribe( const QList<int> & events);

    static void setUid(   int uid );

    static void addJobId( int jId );
    static void delJobId( int jId );

    static void someJobAdded();
    static void someJobDone();
    static void someJobError();

    static WndList* opened[WLAST];

    static void raiseWindow( QWidget * wnd, const QString * name = NULL);

    static void repaint();
    static void repaintStart();
    static void repaintFinish();

	static void startProcess( const QString & cmd, const QString & wdir = QString());

    inline static const QPixmap * getServiceIconLarge( const QString & service_name) { return ms_services_icons_large.value( service_name, NULL);}
    inline static const QPixmap * getServiceIconSmall( const QString & service_name) { return ms_services_icons_small.value( service_name, NULL);}

    void static refreshGui();

    void static loadImage( QPixmap & o_pixmap, const QString & i_filename);

	void static browseImages( const QString & i_image, const QString & i_wdir);

private:
    static MonitorHost * ms_m;
    static Dialog * ms_d;
    static QApplication * ms_app;

    static QLinkedList<Wnd*> ms_windows;
    static QLinkedList<Reciever*> ms_recievers;

    static QStringList ms_previewcmds;
    static QStringList ms_rendercmds;

    static QLinkedList<int> ms_listenjobids;
    static QLinkedList<int> ms_watchtasksjobids;
    static QLinkedList<QWidget*> ms_watchtaskswindows;

    static QMap<QString, QPixmap *> ms_services_icons_large;
    static QMap<QString, QPixmap *> ms_services_icons_small;
};
