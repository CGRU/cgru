#pragma once

#include "../libafanasy/msgclasses/msgclassuserhost.h"

#include "../libafqt/qserver.h"
#include "../libafqt/qthreadclientsend.h"
#include "../libafqt/qthreadclientup.h"

#include "infoline.h"
#include "watch.h"

#include <QtGui/QBoxLayout>
#include <QtCore/QLinkedList>
#include <QtGui/QListWidget>
#include <QtCore/QTimer>
#include <QtGui/QWidget>

class ButtonOut;
class ButtonMonitor;
class ListItems;
class OfflineScreen;
class LabelVersion;

class QHBoxLayout;
class QVBoxLayout;
class QScrollArea;

class Dialog : public QWidget
{
    Q_OBJECT

public:
    Dialog();
    ~Dialog();

    inline bool isInitialized() const { return m_initialized; }
    inline bool isConnected()   const { return m_connected;   }

    inline int getUid() const { return m_uid;}

    void inline displayInfo(    const QString &message) { m_infoline->displayInfo(    message); }
    void inline displayWarning( const QString &message) { m_infoline->displayWarning( message); }
    void inline displayError(   const QString &message) { m_infoline->displayError(   message); }

    void sendMsg( af::Msg * msg);

    bool openMonitor( int type, bool open);

    inline MonitorHost * getMonitor() { return m_monitor;}

    void repaintStart( int mseconds = 1000);
    void repaintFinish();

    void reloadImages();

    void keyPressEvent(    QKeyEvent         * event);

signals:
    void stop();

private slots:
    void newMessage( af::Msg * msg);
    void connectionLost();
    void repaintWatch();

    void actColors();
    void actSounds();
    void actSavePreferencesOnExit();
    void actSaveGUIOnExit();
    void actSaveWndRectsOnExit();
    void actSavePreferences();
    void actShowOfflineNoise();
    void actGuiTheme( QString theme);

protected:
    void contextMenuEvent( QContextMenuEvent * event);
    void closeEvent(       QCloseEvent       * event);
    void paintEvent ( QPaintEvent * event );

private:
    void sendRegister();
    void closeList();
    void connectionEstablished();
    void setDefaultWindowTitle();

private:
    bool m_initialized;
    bool m_connected;
    int  m_uid;
    MonitorHost * m_monitor;

    int m_monitorType;

    afqt::QThreadClientUp   m_qThreadClientUpdate;
    afqt::QThreadClientSend m_qThreadSend;
    afqt::QServer           m_qServer;

    ListItems * m_listitems;
    OfflineScreen * m_offlinescreen;
    InfoLine * m_infoline;
    LabelVersion * m_labelversion;

    QHBoxLayout * m_hlayout_a;
    QVBoxLayout * m_vlayout_a;
    QHBoxLayout * m_hlayout_b;
    QVBoxLayout * m_vlayout_b;

    QTimer m_repaintTimer;

    ButtonMonitor * m_btnMonitor[Watch::WLAST];

    ButtonOut * m_btn_out_left;
    ButtonOut * m_btn_out_right;

    af::MsgClassUserHost m_mcuserhost;

    QPixmap m_img_top;
    QPixmap m_img_topleft;
    QPixmap m_img_topright;
    QPixmap m_img_bot;
    QPixmap m_img_botleft;
    QPixmap m_img_botright;

    QWidget * m_topleft;
    QWidget * m_topright;

    static int ms_size_border_top;
    static int ms_size_border_bot;
    static int ms_size_border_left;
    static int ms_size_border_right;
};
