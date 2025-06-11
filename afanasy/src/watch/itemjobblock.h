#pragma once

#include "../libafanasy/blockdata.h"
#include "../libafanasy/name_af.h"

#include "blockinfo.h"
#include "item.h"

class ListTasks;

class ItemJobBlock : public Item
{
  public:
	ItemJobBlock(const af::BlockData *i_block, ListTasks *i_listtasks);
	~ItemJobBlock();

	void update(const af::BlockData *block, int type);

	inline int getNumBlock() const { return numblock; }

	virtual const QVariant v_getToolTip() const { return m_tooltip; }

	const std::string &getWDir();
	const std::string &getWDirOriginal() const { return m_wdir_orig; }

	inline bool hasFiles() const { return m_files_orig.size(); }
	const std::vector<std::string> &getFiles();
	const std::vector<std::string> &getFilesOriginal() const { return m_files_orig; }

	uint32_t state;
	QString command;
	QString cmdpre;
	QString cmdpost;
	QString tasksname;
	QString service;
	QString parser;
	bool varcapacity;
	bool multihost;
	bool multuhost_samemaster;
	QString multihost_service;

	bool numeric;	   ///< Whether the block is numeric.
	long long first;   ///< First tasks frame.
	long long last;	   ///< Last tasks frame.
	long long pertask; ///< Tasks frames per task.
	long long inc;	   ///< Tasks frames increment.

	int32_t job_id;
	int32_t numblock;

	QString description;

	bool tasksHidden;

	inline const BlockInfo *getInfo() const { return m_info; }

	virtual bool v_mousePressed(int i_x, int i_y, int i_w, int i_h, const Qt::MouseButtons &i_buttons);

	enum SortType
	{
		SNULL = 0,
		SHost,
		SStarts,
		SErrors,
		SPct,
		STime,
		SState
	};

	inline int getSortType() const { return m_sort_type; }
	inline bool isSortAsceding() const { return m_sort_ascending; }

	/// Return old sorting type:
	inline int resetSortingParameters()
	{
		int value = m_sort_type;
		m_sort_type = 0;
		m_sort_ascending = false;
		return value;
	}

	static const int WHost = 60;
	static const int WStarts = 40;
	static const int WErrors = 40;
	static const int WPct = 50;
	static const int WTime = 50;

  protected:
	virtual void v_paint(QPainter *i_painter, const QRect &i_rect,
						 const QStyleOptionViewItem &i_option) const;

  private:
	static const int HeightHeader;
	static const int HeightFooter;

  private:
	std::string m_wdir_orig;
	std::string m_wdir;
	bool m_wdir_ready;
	std::vector<std::string> m_files_orig;
	std::vector<std::string> m_files;
	bool m_files_ready;

	QString m_tooltip;
	std::string m_tooltip_base;
	std::string m_tooltip_properties;
	std::string m_tooltip_progress;

	BlockInfo *m_info;

	int m_sort_type;
	bool m_sort_ascending;

	ListTasks *m_listtasks;
};
