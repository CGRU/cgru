#include "watch.h"

#include "../include/afanasy.h"

#include "../include/afgui.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "../libafqt/qenvironment.h"

#include "blockinfo.h"
#include "buttonmonitor.h"
#include "dialog.h"
#include "item.h"
#include "itemjob.h"
#include "listtasks.h"
#include "monitorhost.h"
#include "popup.h"
#include "receiver.h"
#include "wndlist.h"
#include "wndlistenjob.h"
#include "wndlistentask.h"
#include "wndtask.h"

#include <QApplication>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtGui/QPixmap>

#if QT_VERSION < 0x060000
#include <QSound>
#else
#include <QSoundEffect>
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const QString Watch::BtnName[WLAST] = {"null", "Farm", "Jobs", "Monitors", "Users", "Work"};
const QString Watch::WndName[WLAST] = {"null", "Farm", "Jobs", "Monitors", "Users", "Work"};
WndList *Watch::opened[WLAST] = {0, 0, 0, 0, 0, 0};

const int Watch::Icons_Size_Large = 48;
const int Watch::Icons_Size_Small = 16;
const int Watch::Icons_Size_Tiny = 10;

QList<Wnd *> Watch::ms_windows;
QList<Receiver *> Watch::ms_receivers;
QList<int> Watch::ms_listenjobids;
QList<int> Watch::ms_watchtasksjobids;
QList<QWidget *> Watch::ms_watchtaskswindows;

QMap<QString, QPixmap *> Watch::ms_services_icons_large;
QMap<QString, QPixmap *> Watch::ms_services_icons_small;
QMap<QString, QPixmap *> Watch::ms_services_icons_tiny;
QMap<QString, QPixmap *> Watch::ms_tickets_icons;

QList<int32_t> Watch::ms_jobs_thumbnail_ids;

QApplication *Watch::ms_app = NULL;
Dialog *Watch::ms_d = NULL;

Watch::Watch(Dialog *pDialog, QApplication *pApplication)
{
	ms_app = pApplication;
	ms_d = pDialog;

	// Load icons:
	QString custom_icons_path = afqt::stoq(af::Environment::getIconsPath());
	QString icons_path = afqt::stoq(af::Environment::getCGRULocation()) + "/icons";

	for (QString path : {custom_icons_path, icons_path})
	{
		// Load services icons:
		loadIcons(ms_services_icons_tiny, path + "/software", Icons_Size_Tiny);
		loadIcons(ms_services_icons_small, path + "/software", Icons_Size_Small);
		loadIcons(ms_services_icons_large, path + "/software", Icons_Size_Large);

		// Load tickets icons:
		loadIcons(ms_tickets_icons, path + "/tickets", Icons_Size_Small);
	}
}

Watch::~Watch()
{
	// Delete icons:
	deleteIcons(ms_services_icons_large);
	deleteIcons(ms_services_icons_small);
	deleteIcons(ms_services_icons_tiny);
	deleteIcons(ms_tickets_icons);
}

void Watch::destroy() { ms_d = NULL; }

bool Watch::isPadawan() { return afqt::QEnvironment::level.n == AFGUI::PADAWAN; }
bool Watch::notPadawan() { return afqt::QEnvironment::level.n != AFGUI::PADAWAN; }
bool Watch::isJedi() { return afqt::QEnvironment::level.n == AFGUI::JEDI; }
bool Watch::isSith() { return afqt::QEnvironment::level.n == AFGUI::SITH; }
bool Watch::notSith() { return afqt::QEnvironment::level.n != AFGUI::SITH; }

void Watch::loadIcons(QMap<QString, QPixmap *> &o_map, const QString &i_path, int i_height)
{
	QDir dir(i_path);
	if (false == dir.exists())
	{
		AF_WARN << "Icons folder '" << afqt::qtos(i_path) << "' does not exist.";
		return;
	}

	QFileInfoList files = dir.entryInfoList();
	if (files.size() == 0)
	{
		AF_WARN << "Icons folder '" << afqt::qtos(i_path) << "' is empty.";
		return;
	}

	int icons_loaded = 0;
	for (int i = 0; i < files.size(); i++)
	{
		if (false == files[i].isFile())
			continue;

		QString name = files[i].completeBaseName();

		if (o_map.contains(name))
			continue;

		QPixmap icon(files[i].filePath());
		if (icon.isNull())
		{
			AF_ERR << "Invalid icon: '" << afqt::qtos(files[i].filePath()) << "'.";
			continue;
		}

		o_map[name] = new QPixmap(icon.scaledToHeight(i_height, Qt::SmoothTransformation));

		icons_loaded++;
	}

	if (icons_loaded)
		AF_LOG << "Loaded " << icons_loaded << "*" << i_height << "px icons from '" << afqt::qtos(i_path)
			   << "'.";
	else
		AF_WARN << "No icons loaded from '" << afqt::qtos(i_path) << "'.";
}

void Watch::deleteIcons(QMap<QString, QPixmap *> &o_map)
{
	QMapIterator<QString, QPixmap *> it(o_map);
	while (it.hasNext())
	{
		it.next();
		delete it.value();
	}
	o_map.clear();
}

QWidget *Watch::getWidget() { return (QWidget *)(ms_d); }

void Watch::sendMsg(af::Msg *msg)
{
	if (msg->type() == af::Msg::TJSON)
	{
		msg->setJSONBIN();

		static int unused;
		unused = ::write(1, " <<< ", 5);
		msg->stdOutData(false);
		unused = ::write(1, "\n", 1);
	}
	if (ms_d)
		ms_d->sendMsg(msg);
}

void Watch::get(const std::string &i_str)
{
	std::string str = "{\"get\":{\"binary\":true,";
	str += i_str + "}}";

	af::Msg *msg = af::jsonMsg(str);
	Watch::sendMsg(msg);
}

void Watch::get(const char *i_type, const std::vector<int32_t> &i_ids,
				const std::vector<std::string> &i_modes, const std::vector<int32_t> &i_blocks)
{
	//	{"get":{"type":"jobs","ids":[6],"mode":["progress"],"block_ids":[0]}}
	std::ostringstream str;

	str << "\"type\":\"" << i_type << "\"";

	if (i_ids.size())
	{
		str << ",\"ids\":[";
		for (int i = 0; i < i_ids.size(); i++)
		{
			if (i)
				str << ',';
			str << i_ids[i];
		}
		str << "]";
	}

	if (i_modes.size())
	{
		str << ",\"mode\":[";
		for (int i = 0; i < i_modes.size(); i++)
		{
			if (i)
				str << ',';
			str << '"' << i_modes[i] << '"';
		}
		str << "]";
	}

	if (i_blocks.size())
	{
		str << ",\"block_ids\":[";
		for (int i = 0; i < i_blocks.size(); i++)
		{
			if (i)
				str << ',';
			str << i_blocks[i];
		}
		str << "]";
	}

	get(str.str());
}

void Watch::displayInfo(const QString &message)
{
	if (ms_d)
	{
		ms_d->displayInfo(message);
		if (ms_d->isHidden())
			ms_d->show();
	}
}
void Watch::displayWarning(const QString &message)
{
	if (ms_d)
	{
		ms_d->displayWarning(message);
		if (ms_d->isHidden())
			ms_d->show();
	}
}
void Watch::displayError(const QString &message)
{
	if (ms_d)
	{
		ms_d->displayError(message);
		if (ms_d->isHidden())
			ms_d->show();
	}
}
void Watch::setWindowTitle(const QString &title)
{
	if (ms_d)
	{
		ms_d->setWindowTitle(title);
		if (ms_d->isHidden())
			ms_d->show();
	}
}
void Watch::keyPressEvent(QKeyEvent *event)
{
	if (ms_d)
		ms_d->keyPressEvent(event);
}

bool Watch::isInitialized()
{
	if (ms_d)
		return ms_d->isInitialized();
	else
		return false;
}
bool Watch::isConnected()
{
	if (ms_d)
		return ms_d->isConnected();
	else
		return false;
}

void Watch::addWindow(Wnd *wnd)
{
	if (ms_windows.contains(wnd))
	{
		AFERROR("Watch::addWindow: Window already exists.")
	}
	else
		ms_windows.append(wnd);
}
void Watch::addReceiver(Receiver *receiver)
{
	if (ms_receivers.contains(receiver))
	{
		AFERROR("Watch::addReceiver: Receiver already exists.")
	}
	else
		ms_receivers.append(receiver);
}
void Watch::removeWindow(Wnd *wnd) { ms_windows.removeAll(wnd); }
void Watch::removeReceiver(Receiver *receiver) { ms_receivers.removeAll(receiver); }

void Watch::caseMessage(af::Msg *i_msg)
{
	bool received = false;

	QList<Receiver *>::iterator rIt;
	for (rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
	{
		i_msg->resetWrittenSize();
		if ((*rIt)->v_caseMessage(i_msg) && (false == received))
			received = true;
	}

	i_msg->resetWrittenSize();

	switch (i_msg->type())
	{
		case af::Msg::TMonitorEvents:
		{
			af::MonitorEvents me(i_msg);
			me.v_stdOut();

			// General instructions for the application:
			if (me.m_instruction.size())
			{
				if (me.m_instruction == "exit")
				{
					printf("Received \"exit\" instrucion. Closing dialog.\n");
					ms_d->close();
					return;
				}
			}

			// Let all receivers to process events:
			for (rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
			{
				i_msg->resetWrittenSize();
				if ((*rIt)->v_processEvents(me) && (false == received))
					received = true;
			}

			for (int i = 0; i < me.m_outputs.size(); i++)
			{
				if (WndTask::showTask(me.m_outputs[i]))
					received = true;
			}

			for (int i = 0; i < me.m_listens.size(); i++)
			{
				if (WndTask::showTask(me.m_listens[i]))
					received = true;
			}

			if (me.m_message.size())
			{
				if (LabelVersion::getStringStatus(me.m_message) != LabelVersion::SS_None)
					ms_d->announce(me.m_message);
				else
					new WndText("Message", me.m_message);

				received = true;
			}

			break;
		}
		case af::Msg::TTask:
		{
			af::MCTask mctask(i_msg);
			received = WndTask::showTask(mctask);
			break;
		}
		case af::Msg::TJSON:
		case af::Msg::TJSONBIN:
		{
			rapidjson::Document document;
			std::string error;
			char *data = af::jsonParseMsg(document, i_msg, &error);
			if (data == NULL)
				AF_ERR << error;
			else
			{
				const JSON &infObj = document["info"];
				if (infObj.IsObject())
				{
					std::string text, kind;
					af::jr_string("text", text, infObj);
					af::jr_string("kind", kind, infObj);
					if (kind == "error")
						displayError(afqt::stoq(text));
					else if (kind == "warning")
						displayWarning(afqt::stoq(text));
					else
						displayInfo(afqt::stoq(text));
				}
			}
		}
		case af::Msg::THTTP:
		case af::Msg::THTTPGET:
		{
			static int unused;
			unused = ::write(1, " >>> ", 5);
			i_msg->stdOutData(false);
			unused = ::write(1, "\n", 1);
			received = true;
			break;
		}
	}

	if (false == received)
	{
		printf("Unknown message received: ");
		i_msg->v_stdOut();
	}
}

void Watch::filesReceived(const af::MCTaskUp &i_taskup)
{
	bool found = false;

	for (QList<Receiver *>::iterator rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
	{
		if ((*rIt)->v_filesReceived(i_taskup))
		{
			found = true;
			break;
		}
	}

	if (false == found)
	{
		printf("Watch::filesReceived: Recipient not found:\n");
		i_taskup.v_stdOut();
	}

	ms_jobs_thumbnail_ids.removeAll(i_taskup.getNumJob());
	// printf("Watch::filesReceived: ms_jobs_thumbnail_ids.size = %d\n", ms_jobs_thumbnail_ids.size());
	if (ms_jobs_thumbnail_ids.size())
		getJobThumbnail(ms_jobs_thumbnail_ids.first());

	return;
}

void Watch::queueJobThumbnail(int32_t i_jobid)
{
	// printf("Watch::queueJobThumbnail: ms_jobs_thumbnail_ids.size = %d\n", ms_jobs_thumbnail_ids.size());

	// Check if we are already waiting this job thumbnail
	if (ms_jobs_thumbnail_ids.contains(i_jobid))
		return;

	if (ms_jobs_thumbnail_ids.empty())
		getJobThumbnail(i_jobid);

	ms_jobs_thumbnail_ids.push_back(i_jobid);
}

void Watch::getJobThumbnail(int32_t i_jobid)
{
	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"thumbnail\"";
	str << ",\"ids\":[" << i_jobid << "]";
	str << ",\"binary\":true}}";

	af::Msg *msg = af::jsonMsg(str);
	sendMsg(msg);
}

void Watch::listenJob(int id, const QString &name)
{
	if (ms_listenjobids.contains(id))
		displayWarning("This job is already listening.");
	else
	{
		ms_listenjobids.append(id);
		new WndListenJob(id, name);
	}
}

void Watch::listenTask(int jobid, int block, int task, const QString &name)
{
	new WndListenTask(jobid, block, task, name);
}

void Watch::watchJobTasksWindowAdd(int id, const QString &name)
{
	AFINFA("Watch::watchTasks: trying to open job \"%s\"[%d] tasks window.", name.toUtf8().data(), id)
	QList<int>::const_iterator iIt = ms_watchtasksjobids.begin();
	QList<QWidget *>::iterator wIt = ms_watchtaskswindows.begin();
	while (iIt != ms_watchtasksjobids.end())
	{
		if (*iIt == id)
		{
			raiseWindow(*wIt, &name);
			return;
		}
		iIt++;
		wIt++;
	}

	WndList *wnd = new WndList("Tasks");
	ListTasks *listtasks = new ListTasks(wnd, id, name);
	wnd->setList(listtasks);
	ms_watchtasksjobids.append(id);
	ms_watchtaskswindows.append(wnd);
	displayInfo(QString("Opening '%1' tasks window.").arg(name));
	AFINFA("Watch::watchTasks: \"%s\" window opened.", name.toUtf8().data())
}

void Watch::watchJobTasksWindowRem(int id)
{
	QList<int>::iterator iIt = ms_watchtasksjobids.begin();
	QList<QWidget *>::iterator wIt = ms_watchtaskswindows.begin();
	while (iIt != ms_watchtasksjobids.end())
	{
		if (*iIt == id)
		{
			iIt = ms_watchtasksjobids.erase(iIt);
			wIt = ms_watchtaskswindows.erase(wIt);
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
	for (QList<Receiver *>::iterator rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
		(*rIt)->v_connectionLost();
}

void Watch::connectionEstablished()
{
	for (QList<Receiver *>::iterator rIt = ms_receivers.begin(); rIt != ms_receivers.end(); ++rIt)
		(*rIt)->v_connectionEstablished();
}

bool Watch::openMonitor(int type, bool open)
{
	if (ms_d == NULL)
		return false;
	if (opened[type])
	{
		raiseWindow(opened[type], &WndName[type]);
		return false;
	}
	return ms_d->openMonitor(type, open);
}

void Watch::raiseWindow(QWidget *wnd, const QString *name)
{
	AFINFO("Watch::raiseWindow: trying to raise a window.")
	if ((wnd == NULL) || (name == NULL))
		return;
	if (wnd->isMinimized())
		wnd->showNormal();
	wnd->activateWindow();
	wnd->raise();
	if (name)
		displayInfo(QString("Raising '%1' window.").arg(*name));
	AFINFA("Watch::raiseWindow: \"%s\" window raised.", name->toUtf8().data())
}

void Watch::startProcess(const QString &i_cmd, const QString &i_wdir,
						 const std::map<std::string, std::string> &i_env_map)
{
	printf("Starting '%s'", i_cmd.toUtf8().data());
	if (false == i_wdir.isEmpty())
		printf(" in '%s'", i_wdir.toUtf8().data());
	printf("\n");

#ifdef WINNT
	PROCESS_INFORMATION pinfo;

	char *env = af::processEnviron(i_env_map);
	af::launchProgram(&pinfo, i_cmd.toStdString(), i_wdir.toStdString(), env, NULL, NULL, NULL,
					  CREATE_NEW_CONSOLE, true);

	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
#else
	char **env = af::processEnviron(i_env_map);
	af::launchProgram(i_cmd.toStdString(), i_wdir.toStdString(), env);
#endif

	if (env)
		delete[] env;
}

void Watch::ntf_JobAdded(const ItemJob *i_job)
{
	displayInfo("Job added.");

	Watch::playSound(afqt::QEnvironment::ntf_job_added_sound.str);

	if (afqt::QEnvironment::ntf_job_added_alert.n)
		Watch::notify("Job Added", i_job->getName(), i_job->state);
}

void Watch::ntf_JobDone(const ItemJob *i_job)
{
	displayInfo("Job Done.");

	Watch::playSound(afqt::QEnvironment::ntf_job_done_sound.str);

	if (afqt::QEnvironment::ntf_job_added_alert.n)
		Watch::notify("Job Done", i_job->getName(), i_job->state);
}

void Watch::ntf_JobError(const ItemJob *i_job)
{
	displayWarning("Job Error.");

	Watch::playSound(afqt::QEnvironment::ntf_job_error_sound.str);

	if (afqt::QEnvironment::ntf_job_added_alert.n)
		Watch::notify("Job Error", i_job->getName(), i_job->state);
}

void Watch::repaintStart()
{
	if (ms_d)
		ms_d->repaintStart(100);
}
void Watch::repaintFinish()
{
	if (ms_d)
		ms_d->repaintFinish();
	refreshGui();
}

void Watch::refreshGui()
{
	// Calculate star points:
	Item::calcutaleStarPoints();

	// Refresh Images:
	ButtonMonitor::refreshImages();
	if (ms_d)
		ms_d->reloadImages();

	// Repaint:
	repaint();
}

void Watch::loadImage(QPixmap &o_pixmap, const QString &i_filename)
{
	// Set Pixmap to empty if needed:
	if (i_filename.isEmpty())
	{
		if (false == o_pixmap.isNull())
		{
			o_pixmap = QPixmap();
		}
	}
	else
	{
		// Try to load a file:
		if (false == o_pixmap.load(i_filename))
		{
			// Try to load a file in current theme folder:
			if (false == o_pixmap.load(afqt::stoq(af::Environment::getAfRoot()) + "/icons/watch/" +
									   afqt::QEnvironment::theme.str + "/" + i_filename))
			{
				// Load fails:
				if (false == o_pixmap.isNull())
				{
					// Set Pixmap to empty if it was not
					o_pixmap = QPixmap();
				}
			}
		}
	}
}

void Watch::browseImages(const QString &i_image, const QString &i_wdir)
{
	if (i_image.isEmpty())
		return;

	QString folder = i_image.left(i_image.lastIndexOf('/'));
	folder = folder.left(i_image.lastIndexOf('\\'));
	if (folder == i_image)
	{
		folder = i_wdir;
	}

	Watch::browseFolder(folder, i_wdir);
}

void Watch::browseFolder(const QString &i_folder, const QString &i_wdir)
{
	QDir dir(i_wdir);
	if (dir.exists())
		dir.cd(i_folder);
	else
		dir.setPath(i_folder);

	if (false == dir.exists())
	{
		Watch::displayError(QString("Folder '%1' does not exist.").arg(dir.path()));
		return;
	}

	Watch::displayInfo(QString("Opening '%1'").arg(dir.path().toUtf8().data()));

	QString cmd = afqt::stoq(af::Environment::getOpenFolderCmd());
	cmd = cmd.replace("@PATH@", i_folder);

	Watch::startProcess(cmd, i_wdir);
}

void Watch::openTerminal(const QString &i_wdir)
{
	QDir dir(i_wdir);
	if (false == dir.exists())
	{
		Watch::displayError(QString("Folder '%1' does not exist.").arg(dir.path()));
		return;
	}

	Watch::displayInfo(QString("Terminal '%1'").arg(dir.path()));
#ifdef WINNT
	QString cmd = QString("start cmd.exe /K \"cd %1\"").arg(dir.path());
#else
	QString cmd = QString("cd \"%1\"; openterminal").arg(dir.path());
#endif
	Watch::startProcess(cmd);
}

void Watch::repaint()
{
	// printf("Watch::repaint: start\n");
	QPalette palette = ms_app->palette();
	afqt::QEnvironment::setPalette(palette);
	afqt::QEnvironment::initFonts();
	ms_app->setPalette(palette);

	if (ms_d)
		ms_d->repaint();
	for (int i = 0; i < WLAST; i++)
		if (opened[i])
			opened[i]->repaintItems();
	for (QList<Wnd *>::iterator wIt = ms_windows.begin(); wIt != ms_windows.end(); wIt++)
		(*wIt)->update();
	// printf("Watch::repaint: finish\n");
}

void Watch::notify(const QString &i_title, const QString &i_msg, uint32_t i_state)
{
	new Popup(i_title, i_msg, i_state);
}
void Watch::playSound(const QString &i_file)
{
	if (i_file.isEmpty())
		return;

#if QT_VERSION < 0x060000
	QSound::play(afqt::QEnvironment::ntf_job_added_sound.str);
#else
	QSoundEffect effect;
	effect.setSource(QUrl::fromLocalFile(i_file));
	effect.play();
#endif
}

void Watch::showDocs() { Watch::startProcess("documentation \"afanasy/watch.html\""); }
void Watch::showForum() { Watch::startProcess("forum \"watch\""); }
