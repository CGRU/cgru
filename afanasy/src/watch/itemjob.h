#pragma once

#include "../libafanasy/job.h"

#include "itemnode.h"
#include "blockinfo.h"

class ListJobs;

class ItemJob : public ItemNode
{
public:
	ItemJob( ListJobs * i_list, af::Job *job, bool i_subscibed, const CtrlSortFilter * i_ctrl_sf);
	~ItemJob();

	void updateValues( af::Node *node, int type);

	inline int getErrorRetries(         int block = 0 ) const
		{if(block<m_blocks_num )return m_blockinfo[ block].getErrorsRetries();       else return 0; }
	inline int getErrorsAvoidHost(      int block = 0 ) const
		{if(block<m_blocks_num )return m_blockinfo[ block].getErrorsAvoidHost();     else return 0; }
	inline int getErrorsTaskSameHost(   int block = 0 ) const
		{if(block<m_blocks_num )return m_blockinfo[ block].getErrorsTaskSameHost();  else return 0; }
	inline uint32_t getTasksMaxRunTime( int block = 0 ) const
		{if(block<m_blocks_num )return m_blockinfo[ block].getTasksMaxRunTime();     else return 0; }

	int maxrunningtasks;
	int maxruntasksperhost;
	int64_t state;
	int64_t time_creation;
	int64_t time_started;
	int64_t time_wait;
	int64_t time_done;
	uint32_t time_run;
	int lifetime;

	bool ppapproval;
	bool maintenance;
	bool ignorenimby;
	bool ignorepaused;

	QString service;
	QString hostname;
	QString username;
	QString hostsmask;
	QString hostsmask_exclude;
	QString dependmask;
	QString dependmask_global;
	QString need_os;
	QString need_properties;
	QString cmd_pre;
	QString cmd_post;
	QString description;
	QString thumb_path;
	QString report;
	QString project;
	QString department;
	QMap<QString,QString> folders;

	const QString getRulesFolder();

	inline int getBlocksNum() const { return m_blocks_num;}
	inline int getBlockPercent( int block ) const
		{ if( block < m_blocks_num ) return m_blockinfo[block].p_percentage; else return 0;}

	void setSortType(   int type1, int i_type2 );
	void setFilterType( int type );

	void generateMenu( int id_block, QMenu * menu, QWidget * qwidget);

	bool blockAction( std::ostringstream & i_str, int id_block, const QString & i_action, ListItems * listitems) const;

	inline const QString & getBlockName( int num) const { return m_blockinfo[num].getName();}

	bool calcHeight();

	virtual void v_filesReceived( const af::MCTaskUp & i_taskup);

	void getThumbnail() const;

protected:
	void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
	static const int Height;
	static const int HeightThumbName;
	static const int HeightAnnotation;

private:
	ListJobs * m_list;

	int m_blocks_num;
	bool compact_display;

	QString properties;

	QStringList blocksinfo;
	QString user_eta;

	QString runningTime;

	int     num_runningtasks;
	QString num_runningtasks_str;

	int m_tasks_done;

	QList<QImage*> m_thumbs;
	QList<QString> m_thumbs_paths;

	int block_height;
	BlockInfo * m_blockinfo;
};
