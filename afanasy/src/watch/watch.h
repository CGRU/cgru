#pragma once

#include "../libafqt/name_afqt.h"
#include "../libafqt/qenvironment.h"

#include <QtCore/QList>
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
	Watch(Dialog *pDialog, QApplication *pApplication);
	~Watch();

	static void destroy();

	enum MonType
	{
		WNONE,

		WFarm,
		WJobs,
		WMonitors,
		WUsers,
		WWork,

		WLAST
	};

	static bool isPadawan();
	static bool notPadawan();
	static bool isJedi();
	static bool isSith();
	static bool notSith();

	static const QString BtnName[WLAST];
	static const QString WndName[WLAST];

	inline static Dialog *getDialog() { return ms_d; }
	static QWidget *getWidget();

	static bool isInitialized();
	static bool isConnected();

	static void connectionLost();
	static void connectionEstablished();

	static void sendMsg(af::Msg *msg);

	static void get(const std::string &i_str);
	static void get(const char *i_type, const std::vector<int32_t> &i_ids,
					const std::vector<std::string> &i_modes = std::vector<std::string>(),
					const std::vector<int32_t> &i_blocks = std::vector<int32_t>());

	static void addWindow(Wnd *wnd);
	static void removeWindow(Wnd *wnd);
	static void addReceiver(Receiver *receiver);
	static void removeReceiver(Receiver *receiver);

	static void caseMessage(af::Msg *i_msg);

	static void filesReceived(const af::MCTaskUp &i_taskup);

	static void queueJobThumbnail(int32_t i_jobid);
	static void getJobThumbnail(int32_t i_jobid);

	static void setWindowTitle(const QString &title);

	static void displayInfo(const QString &message);
	static void displayWarning(const QString &message);
	static void displayError(const QString &message);

	static void showDocs();
	static void showForum();

	static bool openMonitor(int type, bool open);

	static void listenJob(int id, const QString &name);
	inline static void listenJob_rem(int id) { ms_listenjobids.removeAll(id); }
	static void watchJobTasksWindowAdd(int id, const QString &name);
	static void watchJobTasksWindowRem(int id);
	static void listenTask(int jobid, int block, int task, const QString &name);

	static void keyPressEvent(QKeyEvent *event);

	static void ntf_JobAdded(const ItemJob *i_job);
	static void ntf_JobDone(const ItemJob *i_job);
	static void ntf_JobError(const ItemJob *i_job);

	static WndList *opened[WLAST];

	static void raiseWindow(QWidget *wnd, const QString *name = NULL);

	static void repaint();
	static void repaintStart();
	static void repaintFinish();

	static void
	startProcess(const QString &i_cmd, const QString &i_wdir = QString(),
				 const std::map<std::string, std::string> &i_env_map = std::map<std::string, std::string>());

	static const int Icons_Size_Large;
	static const int Icons_Size_Small;
	static const int Icons_Size_Tiny;
	inline static const QPixmap *getServiceIconLarge(const QString &i_name)
	{
		return ms_services_icons_large.value(i_name, NULL);
	}
	inline static const QPixmap *getServiceIconSmall(const QString &i_name)
	{
		return ms_services_icons_small.value(i_name, NULL);
	}
	inline static const QPixmap *getServiceIconTiny(const QString &i_name)
	{
		return ms_services_icons_tiny.value(i_name, NULL);
	}
	inline static const QPixmap *getTicketIcon(const QString &i_name)
	{
		return ms_tickets_icons.value(i_name, NULL);
	}

	void static refreshGui();

	void static loadImage(QPixmap &o_pixmap, const QString &i_filename);

	void static browseImages(const QString &i_image, const QString &i_wdir);
	void static browseFolder(const QString &i_folder, const QString &i_wdir = QString());
	void static openTerminal(const QString &i_wdir = QString());

	void static notify(const QString &i_title, const QString &i_msg = QString(), uint32_t i_state = 0);
	void static playSound(const QString &i_file);

  private:
	void loadIcons(QMap<QString, QPixmap *> &o_map, const QString &i_path, int i_height);
	void deleteIcons(QMap<QString, QPixmap *> &o_map);

  private:
	static Dialog *ms_d;
	static QApplication *ms_app;

	static QList<Wnd *> ms_windows;
	static QList<Receiver *> ms_receivers;

	static QList<int> ms_listenjobids;
	static QList<int> ms_watchtasksjobids;
	static QList<QWidget *> ms_watchtaskswindows;

	static QMap<QString, QPixmap *> ms_services_icons_large;
	static QMap<QString, QPixmap *> ms_services_icons_small;
	static QMap<QString, QPixmap *> ms_services_icons_tiny;
	static QMap<QString, QPixmap *> ms_tickets_icons;

	static QList<int32_t> ms_jobs_thumbnail_ids;
};
