#pragma once

#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/name_af.h"
#include "../libafanasy/taskprogress.h"

#include "receiver.h"
#include "wnd.h"

#include <QPushButton>
#include <QTabWidget>

class QCloseEvent;
class QContextMenuEvent;

class ButtonDblClick;
class ListTasks;
class QAfTextWidget;

/// This class designed to request and show any task information

class WndTask : public Wnd
{
	Q_OBJECT
protected:
	WndTask( const af::MCTaskPos & i_tp, ListTasks * i_parent);

public:
	~WndTask();

	/// Static function will try to find task window.
	/// If founded, it will be raised.
	// If not it opens one.
	static WndTask * openTask( const af::MCTaskPos & i_tp, ListTasks * i_parent = NULL);
	/// ( all WndTask windows are stored in a static array )


	/// Static function will try to find task window.
	/// And if founded, it will ask to show info.
	static bool showTask( af::MCTask & i_mctask);


	/// Should be called in parent dtor to close this window.
	void parentClosed();


	/// Check task position:
	inline bool isSameTask( const af::MCTask & i_mctask ) const { return i_mctask.isSameTask( m_pos ); }
	inline bool isSameTask( const af::MCTaskPos & i_tp  ) const { return     i_tp.isSameTask( m_pos ); }


	/// Update task progress.
	/// Called from tasks list ( it receives task progresses updates )
	void updateProgress( const af::TaskProgress & i_progress);

protected:
	virtual void closeEvent( QCloseEvent * i_evt);

private:

	void createTab( const QString & i_name, QWidget ** o_tab, QAfTextWidget ** o_te);

	bool show( af::MCTask & i_mctask);

	void setTaskTitle( const af::MCTask & i_mctask);

	void getTaskInfo( const std::string & i_mode, int i_number = -1) const;

	void taskOperation( const std::string & i_type) const;

	void listen( bool i_subscribe);

	void showExec(             af::MCTask & i_mctask);
	void showOutput(     const af::MCTask & i_mctask);
	void showLog(        const af::MCTask & i_mctask);
	void showErrorHosts( const af::MCTask & i_mctask);
	void showListen(     const af::MCTask & i_mctask);

private slots:
	void slot_currentChanged( int i_index);
	void slot_skip();
	void slot_restart();
	void slot_outputChanged( int i_index);
	void slot_refresh();

private:
	ListTasks * m_parent;

	af::MCTaskPos m_pos;

	af::TaskProgress m_progress;
	QTextEdit * m_progress_te;

	ButtonDblClick * m_btn_skip;
	ButtonDblClick * m_btn_restart;
	QPushButton * m_btn_output;

	QMenu * m_output_menu;
	int m_outputs_count;
	int m_output_current;

	QTabWidget * m_tab_widget;

	QWidget * m_tab_exec;
	QWidget * m_tab_output;
	QWidget * m_tab_log;
	QWidget * m_tab_errhosts;
	QWidget * m_tab_listen;

	QAfTextWidget * m_log_te;
	QAfTextWidget * m_errhosts_te;
	QAfTextWidget * m_output_te;
	QAfTextWidget * m_listen_te;

	QWidget * m_tab_current;

	bool m_listening;

private:
	static std::vector<WndTask*> ms_wndtasks;
};


class ButtonMenu : public QPushButton
{
Q_OBJECT
public:
	ButtonMenu( const QString & i_file, const QString & i_wdir,
			const std::map<std::string,std::string> i_env,
			QWidget * i_parent);

public slots:
	void pushed_slot();
	void launchCmd( int i_index);

protected:
	void contextMenuEvent( QContextMenuEvent * i_event);

private:
	QString m_file;
	QString m_wdir;
	std::map<std::string,std::string> m_env;
	QStringList m_labels;
	QStringList m_cmds;
};

