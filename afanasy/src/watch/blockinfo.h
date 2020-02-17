#pragma once

#include "../include/afjob.h"

#include "../libafanasy/name_af.h"

#include "item.h"

#include <QtCore/QMap>
#include <QtGui/QPainter>

class QStyleOptionViewItem;
class QMenu;

class ListItems;
class Param;

class BlockInfo: public QObject
{
	Q_OBJECT

public:
	BlockInfo(const af::BlockData * i_data, Item * i_item, ListItems * i_listitems);
	~BlockInfo();

	inline const QString & getName() const {return m_name;}

	bool update( const af::BlockData* block, int type);

	void paint( QPainter * i_painter, const QStyleOptionViewItem &option,
		int x, int y, int w,
		bool i_compact_display = false,
		const QColor * i_backcolor = NULL ) const;

	static const int Height;
	static const int HeightCompact;

	inline const QList<Param*> & getParamsList() const {return m_params; }

	void generateMenu(QMenu * i_menu, QMenu * i_params_submenu = NULL) const;

	int p_percentage;
	int p_tasksready;
	int p_tasksrunning;
	int p_tasksdone;
	int p_taskserror;
	int p_tasksskipped;
	int p_taskswarning;
	int p_taskswaitrec;
	int p_avoidhosts;
	int p_errorhosts;
	long long p_capacitytotal;
	long long p_taskssumruntime;

//private:
	uint32_t state;


	QString service;

	QString str_info;
	QString str_params;
	QString str_progress;
	QString str_avoiderrors;

	int tasksnum;

	bool numeric;              ///< Whether the block is numeric.
	bool varcapacity;
	bool multihost;
	bool multihost_samemaster;

	long long frame_first;     ///< First tasks frame.
	long long frame_last;      ///< Last tasks frame.
	long long frame_pertask;   ///< Tasks frames per task.
	long long frame_inc;       ///< Tasks frames increment.
	long long sequential;

	int errors_retries;
	int errors_avoid_host;
	int errors_task_same_host;
	int errors_forgive_time;
	int task_progress_change_timeout;
	int task_max_run_time;
	int task_min_run_time;

	char progress[AFJOB::ASCII_PROGRESS_LENGTH];

	int     capacity;
	int     max_running_tasks;
	int     max_running_tasks_per_host;
	int     need_memory;
	int     need_power;
	int     need_hdd;
	QString need_properties;

	long long filesize_min;
	long long filesize_max;
	int  capcoeff_min;
	int  capcoeff_max;
	uint8_t  multihost_min;
	uint8_t  multihost_max;
	uint16_t multihost_waitmax;
	uint16_t multihost_waitsrv;

	QString depend_mask;
	QString tasks_depend_mask;
	QString hosts_mask;
	QString hosts_mask_exclude;

	QString depends;

	QMap<QString, int> tickets;

signals:
	void sig_BlockAction(int, QString);

private slots:
	void slot_BlockOperation(QString i_operation);
	void slot_BlockChangeParam(const Param * i_param);
	void slot_BlockTicketAdd();
	void slot_BlockTicketEdit(QString i_name);

private:
	void addParam_separator();
	void addParam_Num(const QString & i_name, const QString & i_label, const QString & i_tip, int i_min = -1, int i_max = 1<<30);
	void addParam_Str(const QString & i_name, const QString & i_label, const QString & i_tip);
	void addParam_REx(const QString & i_name, const QString & i_label, const QString & i_tip);
	void addParam_Hrs(const QString & i_name, const QString & i_label, const QString & i_tip);

	void drawProgress(
		QPainter * i_painter,
		int posx, int posy, int width, int height,
		const QColor * i_backcolor = NULL ) const;

	void stdOutFlags( char* data, int size) const;
	void refresh();

private:
	ListItems * m_listitems;
	Item      * m_item;

	int     m_blocknum;
	int     m_jobid;
	QString m_name;

	QList<Param*> m_params;

	QMap<QString, QVariant> m_var_map;

	const QPixmap * m_icon_large;
	const QPixmap * m_icon_small;
};
