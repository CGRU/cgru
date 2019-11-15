#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/blockdata.h"

#include "item.h"
#include "blockinfo.h"

class ListTasks;

class ItemJobBlock : public Item
{
public:
   ItemJobBlock( const af::BlockData* block, ListTasks * list);
   ~ItemJobBlock();

   virtual QSize sizeHint( const QStyleOptionViewItem &option) const;

   void update( const af::BlockData* block, int type);

   inline int getNumBlock() const { return numblock; }

   virtual const QVariant v_getToolTip() const { return m_tooltip;}

	const std::string & getWDir();
	const std::string & getWDirOriginal() const {return m_wdir_orig;}

	inline bool hasFiles() const {return m_files_orig.size();}
	const std::vector<std::string> & getFiles();
	const std::vector<std::string> & getFilesOriginal() const {return m_files_orig;}

   uint32_t state;
   QString  command;
   QString  cmdpre;
   QString  cmdpost;
   QString  tasksname;
   QString  service;
   QString  parser;
   bool     varcapacity;
   bool     multihost;
   bool     multuhost_samemaster;
   QString  multihost_service;

   bool numeric;        ///< Whether the block is numeric.
   long long first;     ///< First tasks frame.
   long long last;      ///< Last tasks frame.
   long long pertask;   ///< Tasks frames per task.
   long long inc;       ///< Tasks frames increment.

	int32_t job_id;
	int32_t numblock;

   QString description;

   static const int ItemId = 1;

   bool tasksHidded;

	inline void generateMenu( int id_block, QMenu * menu, QWidget * qwidget, QMenu * submenu = NULL)
			{ info.generateMenu( id_block, menu, qwidget, submenu);}


	inline bool blockAction( std::ostringstream & i_str, int id_block, const QString & i_action, ListItems * listitems) const
		{ return info.blockAction( i_str, id_block, i_action, listitems);}

   bool mousePressed( const QPoint & pos,const QRect & rect);

   enum SortType
   {
      SNULL = 0,
      SHost,
      SStarts,
      SErrors,
      STime,
      SState
   };

   inline int  getSortType()     const { return sort_type;     }
   inline bool isSortAsceding()  const { return sort_ascending;}

   /// Return old sorting type:
   inline int resetSortingParameters() { int value = sort_type; sort_type = 0; sort_ascending = false; return value;}

   static const int WHost = 60;
   static const int WStarts = 40;
   static const int WErrors = 40;
   static const int WTime = 50;

protected:
	virtual void v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const;
   
private:
   static const int HeightHeader;
   static const int HeightFooter;

private:
   QString m_tooltip;

	std::string m_wdir_orig;
	std::string m_wdir;
	bool m_wdir_ready;
	std::vector<std::string> m_files_orig;
	std::vector<std::string> m_files;
	bool m_files_ready;

   std::string tooltip_base;
   std::string tooltip_properties;
   std::string tooltip_progress;

   int height;
   int width;

   QString  blockToolTip;

   BlockInfo info;

   bool sort_ascending;
   int sort_type;

   ListTasks * listtasks;
};
