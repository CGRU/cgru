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

/// This class designed to request and show any task information

class WndTask : public Wnd
{
	Q_OBJECT
public:
	WndTask( const af::MCTaskPos & i_tp);
	~WndTask();

	/// This is the one public interface.
	/// Static function will try to find task window.
	/// And if founded, it will ask to show info.
	static bool showTask( af::MCTask & i_mctask);
	/// ( all WndTask windows are stored in a static array )

protected:
	virtual void closeEvent( QCloseEvent * i_evt);

private:
	inline bool isSameTask( const af::MCTask & i_mctask) const { return i_mctask.isSameTask( m_pos); }

	void createTab( const QString & i_name, QWidget ** o_tab, QTextEdit ** o_te);

	bool show( af::MCTask & i_mctask);

	void setTaskTitle( const af::MCTask & i_mctask);

	void updateProgress( const af::TaskProgress & i_progress);

	void getTaskInfo( const std::string & i_mode, int i_number = -1);

	void listen( bool i_subscribe);

	void showExec(             af::MCTask & i_mctask);
	void showOutput(     const af::MCTask & i_mctask);
	void showLog(        const af::MCTask & i_mctask);
	void showErrorHosts( const af::MCTask & i_mctask);
	void showListen(     const af::MCTask & i_mctask);

private slots:
	void slot_currentChanged( int i_index);

private:
	af::MCTaskPos m_pos;

	af::TaskProgress m_progress;
	QTextEdit * m_progress_te;

	QTabWidget * m_tab_widget;

	QWidget * m_tab_exec;
	QWidget * m_tab_output;
	QWidget * m_tab_log;
	QWidget * m_tab_errhosts;
	QWidget * m_tab_listen;

	QTextEdit * m_log_te;
	QTextEdit * m_errhosts_te;
	QTextEdit * m_output_te;
	QTextEdit * m_listen_te;

	QWidget * m_tab_current;

	bool m_listening;

private:
	static std::vector<WndTask*> ms_wndtasks;
};


class ButtonMenu : public QPushButton
{
Q_OBJECT
public:
	ButtonMenu( const QString & i_file, const QString & i_wdir, QWidget * i_parent);

public slots:
	void pushed_slot();
	void launchCmd( int i_index);

protected:
	void contextMenuEvent( QContextMenuEvent * i_event);

private:
	QString m_file;
	QString m_wdir;
	QStringList m_labels;
	QStringList m_cmds;
};

