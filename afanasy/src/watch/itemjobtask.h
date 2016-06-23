#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/taskprogress.h"

#include "item.h"
#include "itemjobblock.h"

#include <QtGui/QImage>

class ListTasks;

class ItemJobTask : public Item
{
public:

	/// main ctor, used when adding a task to a ListTask
	ItemJobTask( ListTasks * i_list, const ItemJobBlock * i_block, int i_numtask, const af::BlockData * i_bdata);

	~ItemJobTask();

	virtual bool calcHeight();

	void upProgress( const af::TaskProgress & tp);

	inline bool isBlockNumeric() const { return m_block->numeric;}

	inline int getBlockNum() const { return m_blocknum; }
	inline int getTaskNum()  const { return m_tasknum;  }

	const std::string & getWDir() const;

	inline bool hasFiles() const { return m_files.size(); }
	inline const std::vector<std::string> & getFiles() const { return m_files; }

	inline const long long getFramesNum() const { return m_frames_num; }

	af::TaskProgress taskprogress;

	virtual const QVariant getToolTip() const;
	virtual const QString getSelectString() const;

	void showThumbnail();

	static const int ItemId = 2;
	static const int WidthInfo;

	bool compare( int type, const ItemJobTask & other, bool ascending) const;

	void taskFilesReceived( const af::MCTaskUp & i_taskup );
	
	/// Send a query for information about this task to the server
	void getTaskInfo(const std::string &i_mode, int i_number = -1);

	inline const af::MCTaskPos getTaskPos() const { return af::MCTaskPos( m_job_id, m_blocknum, m_tasknum);}
	
protected:
	virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
	void thumbsCLear();

private:
	static const int TaskHeight = 13;
	static const int TaskThumbHeight = 100;

private:
	ListTasks * m_list;

	int m_job_id;
	int m_blocknum;
	int m_tasknum;
	const ItemJobBlock * m_block;

	long long m_frame_first;
	long long m_frame_last;
	long long m_frames_num;
	std::vector<std::string> m_files;

	int m_thumbs_num;
	QImage ** m_thumbs_imgs;
};
