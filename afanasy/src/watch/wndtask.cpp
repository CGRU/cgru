#include "wndtask.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/service.h"
#include "../libafanasy/taskexec.h"

#include "../libafqt/name_afqt.h"

#include "actionid.h"
#include "listtasks.h"
#include "monitorhost.h"
#include "buttondblclick.h"
#include "qaftextwidget.h"
#include "watch.h"

#include <QtGui/QContextMenuEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTextEdit>
#include "itemjobtask.h"
#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

std::vector<WndTask*> WndTask::ms_wndtasks;

WndTask * WndTask::openTask(const af::MCTaskPos & i_tp,
							ListTasks * i_parent, 
							ItemJobTask* task_ptr)
							{
								std::vector<WndTask*>::iterator it = ms_wndtasks.begin();
								while(it != ms_wndtasks.end())
									if ((*it)->isSameTask(i_tp))
									{
										(*it)->setVisible(true);
										(*it)->showNormal();
										(*it)->raise();
										return (*it);
									}
									else
										it++;

								return new WndTask(i_tp, i_parent, task_ptr);
							}

bool WndTask::showTask(af::MCTask & i_mctask)
{
	std::vector<WndTask*>::iterator it = ms_wndtasks.begin();
	while(it != ms_wndtasks.end())
		if ((*it)->isSameTask(i_mctask))
			return((*it)->show(i_mctask));
		else
			it++;

	return false;
}

WndTask::WndTask(const af::MCTaskPos & i_tp, ListTasks * i_parent, ItemJobTask* task_ptr):
	Wnd("Task"),
	m_parent(i_parent),
	m_pos(i_tp),
	m_log_te(NULL),
	m_errhosts_te(NULL),
	m_output_te(NULL),
	m_outputs_count(-1),
	m_output_current(-1),
	m_tab_current(NULL),
	m_listening(false),
	m_task_ptr(task_ptr)
{
	ms_wndtasks.push_back(this);

	QVBoxLayout * layout = new QVBoxLayout(this);

	//
	// Progess brief:
	//
	m_progress_te = new QTextEdit(this);
	layout->addWidget(m_progress_te);
	m_progress_te->setReadOnly(true);
	m_progress_te->setLineWrapMode(QTextEdit::NoWrap);
	m_progress_te->setFocusPolicy(Qt::NoFocus);

	m_progress_te->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_progress_te->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_progress_te->setFixedHeight(28);


	//
	// Buttons:
	//
	QHBoxLayout * layoutB = new QHBoxLayout();
	layout->addLayout(layoutB);
	layoutB->setSpacing(4);

	m_btn_skip = new ButtonDblClick("Skip", this);
	layoutB->addWidget(m_btn_skip);
	m_btn_skip->setEnabled(false);
	connect(m_btn_skip, SIGNAL(sig_dblClicked()), this, SLOT(slot_skip()));

	m_btn_restart = new ButtonDblClick("Restart", this);
	layoutB->addWidget(m_btn_restart);
	m_btn_restart->setEnabled(false);
	connect(m_btn_restart, SIGNAL(sig_dblClicked()), this, SLOT(slot_restart()));

	layoutB->addStretch();

	m_btn_output = new QPushButton("Output", this);
	layoutB->addWidget(m_btn_output);
	m_btn_output->setEnabled(false);
	m_btn_output->setFixedWidth(111);
	m_output_menu = new QMenu(m_btn_output);
	m_btn_output->setMenu(m_output_menu);


	//
	// Output tabs:
	//
	m_tab_widget = new QTabWidget(this);
	layout->addWidget(m_tab_widget);
	QPushButton * refresh = new QPushButton("refresh");
	m_tab_widget->setCornerWidget(refresh);
	connect(refresh, SIGNAL(pressed()), this, SLOT(slot_refresh()));

	createTab("Executable",  &m_tab_exec,      NULL         );
	createTab("Output",      &m_tab_output,   &m_output_te  );
	createTab("Log",         &m_tab_log,      &m_log_te     );
	createTab("Error Hosts", &m_tab_errhosts, &m_errhosts_te);
	createTab("Listen",      &m_tab_listen,   &m_listen_te  );

	m_tab_output->setEnabled(false);

	connect(m_tab_widget, SIGNAL(currentChanged(int)), this, SLOT(slot_currentChanged(int)));

	getTaskInfo("info");
}

void WndTask::createTab(const QString & i_name, QWidget ** o_tab, QAfTextWidget ** o_te)
{
	*o_tab = new QWidget(m_tab_widget);
	m_tab_widget->addTab(*o_tab, i_name);

	if (o_te)
	{
		QVBoxLayout * layout = new QVBoxLayout(*o_tab);

		*o_te = new QAfTextWidget(*o_tab);
		layout->addWidget(*o_te);
		(*o_te)->setLineWrapMode(QTextEdit::NoWrap);
		(*o_te)->setReadOnly(true);
	}
}

WndTask::~WndTask()
{
	if (m_parent)
		m_parent->taskWindowClosed(this);
}

void WndTask::closeEvent(QCloseEvent * i_evt)
{
	if (m_listening)
		listen(false);

	std::vector<WndTask*>::iterator it = ms_wndtasks.begin();
	while(it != ms_wndtasks.end())
		if (*it == this)
			it = ms_wndtasks.erase(it);
		else
			it++;

	Wnd::closeEvent(i_evt);
}

void WndTask::parentClosed()
{
	m_parent = NULL;
	close();
}

void WndTask::slot_currentChanged(int i_index)
{
	QWidget * tab = m_tab_widget->currentWidget();
	if (tab == m_tab_current) return;

	m_tab_current = tab;

	if (m_tab_current == m_tab_output)
	{
		if (m_output_te->isEmpty())
			getTaskInfo("output", m_output_current);
	}
	else if (m_tab_current == m_tab_log)
	{
		if (m_log_te->isEmpty())
			getTaskInfo("log");
	}
	else if (m_tab_current == m_tab_errhosts)
	{
		if (m_errhosts_te->isEmpty())
			getTaskInfo("error_hosts");
	}
	else if (m_tab_current == m_tab_listen)
		listen(true);
//	else if (m_tab_current == m_tab_exec)
//		getTaskInfo("info");
}

void WndTask::slot_refresh()
{
	m_output_te->clearText();
	m_log_te->clearText();
	m_errhosts_te->clearText();

	m_tab_current = NULL;
	slot_currentChanged(m_tab_widget->currentIndex());
}

void WndTask::getTaskInfo(const std::string & i_mode, int i_number) const
{
	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\"";
	str << ",\"mode\":\"" << i_mode << "\"";
	str << ",\"ids\":[" << m_pos.getJobId() << "]";
	str << ",\"block_ids\":[" << m_pos.getBlockNum() << "]";
	str << ",\"task_ids\":[" << m_pos.getTaskNum() << "]";
	if (i_number != -1)
		str << ",\"number\":" << i_number;
	str << ",\"mon_id\":" << MonitorHost::id();
	str << ",\"binary\":true}}";

	af::Msg * msg = af::jsonMsg(str);
	Watch::sendMsg(msg);
}

void WndTask::slot_skip()    { taskOperation("skip"  ); }
void WndTask::slot_restart() { taskOperation("restart"); }

void WndTask::taskOperation(const std::string & i_type) const
{
	std::ostringstream str;
	af::jsonActionStart(str, "jobs", "", std::vector<int>(1, m_pos.getJobId()));
	str << ",\n\"operation\":{\n\"type\":\"" << i_type << '"';
	str << ",\n\"task_ids\":[" << m_pos.getTaskNum() << "]}";
	str << ",\n\"block_ids\":[" << m_pos.getBlockNum() << ']';

	af::jsonActionFinish(str);
	Watch::sendMsg(af::jsonMsg(str));
}

void WndTask::listen(bool i_subscribe)
{
	if (m_listening == i_subscribe)
		return;

	std::ostringstream str;
	af::jsonActionOperationStart(str,"monitors","watch","", MonitorHost::ids());
	str << ",\"class\":\"listen\"";
	str << ",\"job\":" << m_pos.getJobId();
	str << ",\"block\":" << m_pos.getBlockNum();
	str << ",\"task\":" << m_pos.getTaskNum();
	str << ",\"status\":\"" << (i_subscribe ? "subscribe":"unsubscribe") << "\"";
	af::jsonActionOperationFinish(str);

	Watch::sendMsg(af::jsonMsg(str));

	m_listening = i_subscribe;
}

void WndTask::setTaskTitle(const af::MCTask & i_mctask)
{
	std::ostringstream str;

	str << "Task ";
	str << "[" << i_mctask.m_job_name   + "]";
	str << "[" << i_mctask.m_block_name + "]";
	str << "[" << i_mctask.m_task_name  + "]";

	setWindowTitle(afqt::stoq(str.str()));
}

void WndTask::updateProgress(const af::TaskProgress & i_progress)
{
	// Just copy it for storing:
	m_progress = i_progress;

	//
	// Update brief string:
	//
	std::ostringstream str;

	str << "Status:<b>";
	if (m_progress.state & AFJOB::STATE_READY_MASK  ) str << ' ' << AFJOB::STATE_READY_NAME;
	if (m_progress.state & AFJOB::STATE_RUNNING_MASK) str << ' ' << AFJOB::STATE_RUNNING_NAME;
	if (m_progress.state & AFJOB::STATE_DONE_MASK   ) str << ' ' << AFJOB::STATE_DONE_NAME;
	if (m_progress.state & AFJOB::STATE_SKIPPED_MASK) str << ' ' << AFJOB::STATE_SKIPPED_NAME;
	if (m_progress.state & AFJOB::STATE_ERROR_MASK  ) str << ' ' << AFJOB::STATE_ERROR_NAME;
	str << "</b>";

	if (m_progress.state & AFJOB::STATE_RUNNING_MASK) str << " <b>" << int(m_progress.percent) << "%</b>";
	if (m_progress.starts_count) str << " Starts: <b>" << m_progress.starts_count << "</b>";
	if (m_progress.errors_count) str << " (<b>" << m_progress.errors_count << "</b> errors)";
	if (m_progress.hostname.size()) str << " Last host: <b>" << m_progress.hostname << "</b>";

	if (m_progress.time_start && m_progress.time_done && (false == (m_progress.state & AFJOB::STATE_RUNNING_MASK)))
		str << " Time: <b>" << af::time2strHMS(m_progress.time_done - m_progress.time_start) << "</b>";

	m_progress_te->setHtml(afqt::stoq(str.str()));

	//
	// Update buttons state:
	//

	if ((m_progress.state & AFJOB::STATE_READY_MASK) ||
		(m_progress.state & AFJOB::STATE_RUNNING_MASK))
		m_btn_skip->setEnabled(true);
	else
		m_btn_skip->setEnabled(false);

	if ((m_progress.state & AFJOB::STATE_RUNNING_MASK) ||
		(m_progress.state & AFJOB::STATE_DONE_MASK) ||
		(m_progress.state & AFJOB::STATE_ERROR_MASK))
		m_btn_restart->setEnabled(true);
	else
		m_btn_restart->setEnabled(false);

	//
	// Process outputs count:
	//
	if (m_progress.starts_count)
		m_tab_output->setEnabled(true);

	int outputs_count = m_progress.starts_count;
	if (m_progress.state & AFJOB::STATE_RUNNING_MASK)
		outputs_count -= 1;

	if (outputs_count != m_outputs_count)
	{
		m_output_menu->clear();
		m_outputs_count = outputs_count;

		if (m_outputs_count > 0)
		{
			m_btn_output->setEnabled(true);

			ActionId * action = new ActionId(-1,"Latest", m_output_menu);
			m_output_menu->addAction(action);
			connect(action, SIGNAL(triggeredId(int)), this, SLOT(slot_outputChanged(int)));

			for(int i = 1; i <= m_outputs_count; i++)
			{
				ActionId * action = new ActionId(i, QString("Session #%1").arg(i), m_output_menu);
				m_output_menu->addAction(action);
				connect(action, SIGNAL(triggeredId(int)), this, SLOT(slot_outputChanged(int)));
			}
		}
	}
}

void WndTask::slot_outputChanged(int i_index)
{
	QString label = "Ouput";
	if (i_index > 0)
		label += QString("[#%1]").arg(i_index);
	else
		label += "[-1]";
	m_btn_output->setText(label);

	m_output_current = i_index;

	getTaskInfo("output", m_output_current);
}

bool WndTask::show(af::MCTask & i_mctask)
{
	AF_DEBUG << i_mctask;

	if (false == i_mctask.isSameTask(m_pos))
		return false;

	updateProgress(i_mctask.m_progress);

	setTaskTitle(i_mctask);

	switch(i_mctask.getType())
	{
		case af::MCTask::TExec:
		{
			showExec(i_mctask);
			break;
		}
		case af::MCTask::TOutput:
		{
			showOutput(i_mctask);
			break;
		}
		case af::MCTask::TLog:
		{
			showLog(i_mctask);
			break;
		}
		case af::MCTask::TErrorHosts:
		{
			showErrorHosts(i_mctask);
			break;
		}
		case af::MCTask::TListen:
		{
			showListen(i_mctask);
			break;
		}
		default:
		{
			AF_ERR << "Unknown type: " << i_mctask;
			Watch::displayError("Unknown task message type.");
			return false;
		}
	}

	return true;
}

void WndTask::showExec(af::MCTask & i_mctask)
{
	af::TaskExec * exec = i_mctask.getExec();
	#ifdef AFOUTPUT
	exec->v_stdOut(true);
	#endif

	m_tab_widget->setCurrentWidget(m_tab_exec);

	QVBoxLayout * box = new QVBoxLayout(m_tab_exec);

	QSplitter * splitter = new QSplitter(Qt::Vertical, m_tab_exec);
	box->addWidget(splitter);

	QTextEdit * qTextEdit = new QTextEdit(m_tab_exec);
	splitter->addWidget(qTextEdit);
	qTextEdit->setAcceptRichText(false);
	qTextEdit->setLineWrapMode(QTextEdit::NoWrap);
	qTextEdit->setReadOnly(true);

	// Create task and service:
	af::Service service(exec);
	QString wdir    = afqt::stoq(service.getWDir());
	QString command = afqt::stoq(service.getCommand());
	std::map<std::string,std::string> environment = service.getEnvironment();
	std::vector<std::string> files = service.getFiles();
	std::vector<std::string> parsed_files = service.getParsedFiles();

	// Output task data:
	QTextCharFormat fCommon;
	QTextCharFormat fBold;
	QTextCharFormat fItalic;
	fBold.setFontWeight(QFont::Bold);
	fItalic.setFontItalic(true);

	QTextCursor c(qTextEdit->textCursor());

	c.insertText(afqt::stoq(exec->getName()), fBold);
	c.insertText("\n");
	c.insertText(afqt::stoq(exec->getServiceType()), fBold);
	c.insertText("[", fItalic);
	c.insertText(afqt::stoq(exec->getParserType()), fBold);
	c.insertText("]:", fItalic);
	c.insertText(QString::number(exec->getCapacity()), fBold);
	c.insertText(" frames(", fItalic);
	c.insertText(QString::number(exec->getFrameStart()), fBold);
	c.insertText(",", fItalic);
	c.insertText(QString::number(exec->getFrameFinish()), fBold);
	c.insertText(",", fItalic);
	c.insertText(QString::number(exec->getFramesNum()), fBold);
	c.insertText("):", fItalic);

	c.insertText("\nCommand:\n", fItalic);
	c.insertText(command, fBold);

	c.insertText("\nWorking Directory:\n", fItalic);
	c.insertText(wdir, fBold);

	if (environment.size())
	{
		c.insertText("\nEnvironment:", fItalic);
		for (auto const& x : environment)
		{
			c.insertText(afqt::stoq("\n" + x.first), fBold);
			c.insertText(afqt::stoq("=" + x.second), fCommon);
		}
	}

	if (files.size())
	{
		c.insertText("\n");
		c.insertText("Files:", fItalic);
		for(int i = 0; i < files.size(); i++)
			c.insertText("\n" + afqt::stoq(files[i]), fBold);
	}
/*	if (parsed_files.size())
	{
		c.insertText("\n");
		c.insertText("Parsed Files:", fItalic);
		for(int i = 0; i < parsed_files.size(); i++)
			c.insertText("\n" + afqt::stoq(parsed_files[i]), fBold);
	}*/
	if (exec->hasFileSizeCheck())
	{
		c.insertText("\n");
		c.insertText("File Size Check: ", fItalic);
		c.insertText(QString::number(exec->getFileSizeMin()), fBold);
		c.insertText(" - ", fItalic);
		c.insertText(QString::number(exec->getFileSizeMax()), fBold);
	}

	if (parsed_files.size())
		files = parsed_files;

	if (files.empty())
		return;


	// Create preview files controls:
	QScrollArea * scroll = new QScrollArea();
//	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	QWidget * widget = new QWidget();
	QVBoxLayout * layout = new QVBoxLayout();
	widget->setLayout(layout);
	scroll->setWidget(widget);
	splitter->addWidget(scroll);

	splitter->setSizes(QList<int>() << 300 << (files.size() > 10 ? 300 : files.size() * 30));
	scroll->setWidgetResizable(true);
	layout->setSpacing(0);
	layout->setContentsMargins(1, 1, 1, 1);

	if (parsed_files.size())
		layout->addWidget(new QLabel("Files (parsed):", m_tab_exec));
	else
		layout->addWidget(new QLabel("Files:", m_tab_exec));
	QHBoxLayout * layoutH = new QHBoxLayout();
	QVBoxLayout * layoutL = new QVBoxLayout();
	QVBoxLayout * layoutR = new QVBoxLayout();
	layout ->addLayout(layoutH);
	layoutH->addLayout(layoutL);
	layoutH->addLayout(layoutR);

	for(int i = 0; i < files.size(); i++)
	{
		QLineEdit * fileField = new QLineEdit(m_tab_exec);
		layoutL->addWidget(fileField);
		fileField->setReadOnly(true);
		fileField->setText(afqt::stoq(files[i]));

		ButtonMenu * btnBrowse = new ButtonMenu(afqt::stoq(files[i]), wdir, exec->getEnv(), m_tab_exec,m_task_ptr);
		layoutR->addWidget(btnBrowse);
	}

	delete exec;
}

void WndTask::showOutput(const af::MCTask & i_mctask)
{
	m_tab_widget->setCurrentWidget(m_tab_output);

	af::Service service(i_mctask.m_service, i_mctask.m_parser);

	if (service.isInitialized() && service.hasParser())
	{
		std::string html = service.toHTML(i_mctask.getOutput());
		QString text = afqt::stoq(html);
		text = text.replace("<br>\n","\n");
		text = text.replace("<br>","\n");
		text = QString("<pre>%1</pre>").arg(text);
		m_output_te->setHtml(text);
	}
	else
	{
		m_output_te->setPlainText(afqt::stoq(i_mctask.getOutput()));
	}
}

void WndTask::showLog(const af::MCTask & i_mctask)
{
	m_tab_widget->setCurrentWidget(m_tab_log);

	std::string log = i_mctask.getLog();
	if (log.empty())
	{
		log = "Task log is empty.";
	}

	m_log_te->setPlainText(afqt::stoq(log));
}

void WndTask::showErrorHosts(const af::MCTask & i_mctask)
{
	m_tab_widget->setCurrentWidget(m_tab_errhosts);

	std::string list = i_mctask.getErrorHosts();
	if (list.empty())
	{
		list = "Task has no error hosts.";
	}

	m_errhosts_te->setPlainText(afqt::stoq(list));
}

void WndTask::showListen(const af::MCTask & i_mctask)
{
	m_listen_te->append(afqt::stoq(i_mctask.getListened()));
}


ButtonMenu::ButtonMenu(const QString & i_file, const QString & i_wdir,
		const std::map<std::string,std::string> i_env, QWidget * i_parent,
		ItemJobTask* task_ptr ):
	QPushButton("Launch", i_parent),
	m_file(i_file),
	m_wdir(i_wdir),
	m_env(i_env),
	m_task_ptr(task_ptr)
{
	//CN_OK("ButtonMenu:m_task_ptr =",m_task_ptr);
	std::vector<std::string>::const_iterator it = af::Environment::getPreviewCmds().begin();
	for(; it != af::Environment::getPreviewCmds().end(); it++)
	{
		QString cmd(afqt::stoq(*it));
		QStringList cmdSplit = cmd.split("|");
		
		m_labels.append(cmdSplit.first());
		cmd = cmdSplit.last().replace(AFWATCH::CMDS_ARGUMENT, m_file);
		m_cmds.append(cmd);
	}

	connect(this, SIGNAL(pressed()), this, SLOT(pushed_slot()));
}

void ButtonMenu::pushed_slot()
{
	contextMenuEvent(new QContextMenuEvent(QContextMenuEvent::Mouse, QPoint()));
}

void ButtonMenu::contextMenuEvent(QContextMenuEvent * i_event)
{
	QMenu menu(this);
	ActionId * action;

	action = new ActionId(-1, "Browse", this);
	connect(action, SIGNAL(triggeredId(int)), this, SLOT(launchCmd(int)));
	menu.addAction(action);

	menu.addSeparator();

	for(int i = 0; i < m_cmds.size(); i++)
	{
		action = new ActionId(i, m_labels[i], this);
		connect(action, SIGNAL(triggeredId(int)), this, SLOT(launchCmd(int)));
		menu.addAction(action);
	}
	//============================================================================
	// By Suren
	//============================================================================
	menu.addSeparator();
	if (m_task_ptr != nullptr)
	{
		std::set<std::string> l_all_services;
		auto l_block_ptr 		= m_task_ptr->get_block();
		m_task_info.m_job_id   	= m_task_ptr->m_job_id;
		m_task_info.m_service 	= m_task_ptr->get_block()->service.toStdString();
		m_task_info.m_id  	 	= m_task_ptr->m_tasknum;
		m_task_info.m_block_id  = m_task_ptr->m_blocknum;
		m_task_info.m_done 		= (l_block_ptr->state & AFJOB::STATE_DONE_MASK)>0;	;//(l_block_ptr->time_done > l_block_ptr->time_started);
		bool l_all_done = m_task_info.m_done;
		l_all_services.insert(m_task_info.m_service);
		//create menus
		const auto& l_popups = cn::Pref::get()->get_popups();
		for (const auto& l_popup:l_popups )
		{
			if (l_popup.second->has_stage("task") == false ) continue;
			l_popup.second->reset_env_nan();
			l_popup.second->add_dict_val("STAGE","TASK");

			const auto& l_menus = l_popup.second->get_menus("task");
			if (l_menus.empty() == true) continue;

			QMenu * submenu = new QMenu( QString::fromStdString(l_popup.first), this );
			for (const auto& l_menu: l_menus) 
			{
				auto qaction = new QAction( QString::fromStdString(l_menu->get_name()), this);
				submenu->addAction( qaction);
				// Add lambda as menu funstion
				QObject::connect( qaction,&QAction::triggered, 
					[this, l_menu]() {
						std::string l_cmd = l_menu->get_cmd();
						std::string l_arg = l_menu->get_arg();
						std::vector<std::string> l_cmd_base, l_arg_base;
						std::vector<std::string> l_cmd_sec,  l_arg_sec;
						bool  l_parse_cmd_res = cn::StringParser::parse_to_vectors(l_cmd_base, l_cmd_sec, l_cmd.c_str());
						bool  l_parse_arg_res = cn::StringParser::parse_to_vectors(l_arg_base, l_arg_sec, l_arg.c_str());
						
						if (l_parse_cmd_res == false || l_parse_arg_res == false)
						{
							auto l_src = l_menu->get_popup_menu()->get_source();
							auto l_stg = l_menu->get_stage()->get_name();
							CN_ERR("Failed to parse command and arg");
							CN_WARN("File: ",l_src);
							CN_WARN("Stage: ",l_stg);
							CN_WARN("CMD: ", l_cmd);
							CN_WARN("ARG: ", l_arg);
							return;
						}
						
						l_menu->get_popup_menu()->add_dict_val("JOB_ID",   std::to_string(m_task_info.m_job_id) );
						l_menu->get_popup_menu()->add_dict_val("BLOCK_ID", std::to_string(m_task_info.m_block_id) );
						l_menu->get_popup_menu()->add_dict_val("TASK_ID",  std::to_string(m_task_info.m_id) );
						l_menu->get_popup_menu()->add_dict_val("SERVICE",  m_task_info.m_service);
						std::string l_cmd_solved = "";

						// fill cmd
						l_menu->get_popup_menu()->generate_cmd(l_cmd_base, l_cmd_sec, l_cmd_solved);
						l_cmd_solved += " ";
						l_menu->get_popup_menu()->generate_cmd(l_arg_base, l_arg_sec, l_cmd_solved);
						l_cmd_solved += " ";
						CN_OK(l_cmd_solved);
						QString qcmd = QString::fromStdString(l_cmd_solved);
						Watch::startProcess(qcmd,"");

						l_cmd = "";
						l_arg = "";
				});

				std::string l_ext_str;
				if (l_menu->get_enable_for( false,
											l_all_done,
											l_all_services,
											l_ext_str) == false)
				{
					qaction->setDisabled(true);
					qaction->setText(QString::fromStdString(l_ext_str));
				}
			}
			//submenu->setStyleSheet("QMenu::item:disabled { color: gray !important; padding-left: 20px !important; }");
			submenu->setStyleSheet("QMenu::item:disabled { color: gray !important; }");
			menu.addMenu(submenu);
		}

	}
	menu.exec(i_event->globalPos());
}

void ButtonMenu::launchCmd(int i_index)
{
	if (i_index == -1)
	{
		Watch::browseImages(m_file, m_wdir);
		return;
	}

	if (i_index >= m_cmds.size())
	{
		AFERRAR("ButtonMenu::launchCmd: index is out of range: %d >= %d", i_index, m_cmds.size())
		return;
	}
	
	Watch::startProcess(m_cmds[i_index], m_wdir, m_env);
}


