#pragma once

#include "../libafanasy/regexp.h"

#include "listitems.h"

#include <QAction>
#include <QFrame>

class QLabel;
class QHBoxLayout;

class CtrlSortFilterMenu;
class ListItems;

class CtrlSortFilter : public QFrame
{
Q_OBJECT
public:
	CtrlSortFilter( ListItems * i_parent,
		int * i_sorttype1, bool * i_sortascending1,
		int * i_sorttype2, bool * i_sortascending2,
		int * i_filtertype, bool * i_filterinclude, bool * i_filtermatch, std::string * i_filterstring);
	~CtrlSortFilter();

	inline void addSortType(   int i_type) { if( i_type < TLAST) m_sort_types.push_back(   i_type); }
	inline void addFilterType( int i_type) { if( i_type < TLAST) m_filter_types.push_back( i_type); }

	void init();

	inline bool isSortEnabled()     const { return  (*m_sorttype1 != TNONE ) || (*m_sorttype2 != TNONE );}
	inline int  getSortType1()      const { return   *m_sorttype1;       }
	inline int  getSortType2()      const { return   *m_sorttype2;       }
	inline bool isSortAscending1()  const { return   *m_sortascending1;  }
	inline bool isSortAscending2()  const { return   *m_sortascending2;  }
	inline bool isSortDescending1() const { return !(*m_sortascending1); }
	inline bool isSortDescending2() const { return !(*m_sortascending2); }

	inline bool isFilterEnabled()       const { return   *m_filtertype != TNONE; }
	inline bool isFilterEmpty()         const { return    m_filter->empty();}
	inline int  getFilterType()         const { return   *m_filtertype;     }
	inline bool isFilterInclude()       const { return   *m_filterinclude;  }
	inline bool isFilterExclude()       const { return !(*m_filterinclude); }
	inline bool isFilterMatch()         const { return   *m_filtermatch;    }
	inline bool isFilterContain()       const { return !(*m_filtermatch);   }

	inline const af::RegExp & getFilterRE() const { return m_filter_re; }

	inline QHBoxLayout * getLayout() { return m_layout;}

	enum TYPE{
		TNONE,
		TPRIORITY,
		TCAPACITY,
		TNAME,
		TUSERNAME,
		TTASKUSER,
		THOSTNAME,
		TNUMJOBS,
		TNUMRUNNINGTASKS,
		TSERVICE,
		TTIMECREATION,
		TTIMELAUNCHED,
		TTIMESTARTED,
		TTIMEREGISTERED,
		TTIMEACTIVITY,
		TTIMEFINISHED,
		TTIMERUN,
		TENGINE,
		TADDRESS,
		TELDERTASKTIME,
		TLAST
	};

	static const char * TNAMES[];
	static const char * TNAMES_SHORT[];

signals:
	void sortTypeChanged();
	void sortDirectionChanged();
	void filterChanged();
	void filterTypeChanged();
	void filterSettingsChanged();

protected:
	void contextMenuEvent( QContextMenuEvent *i_event);

private slots:
	void actSortType1( int i_type);
	void actSortType2( int i_type);
	void actSortAscending1();
	void actSortAscending2();
	void actFilter( const QString & i_str);
	void actFilterInclude();
	void actFilterMacth();
	void actFilterType( int i_type);

private:
	void selLabel();

private:

	std::vector<int> m_sort_types;
	std::vector<int> m_filter_types;

	ListItems * m_parernlist;

	QHBoxLayout * m_layout;

	QLabel * m_sort_label;
	QLabel * m_filter_label;

	int  * m_sorttype1;
	int  * m_sorttype2;
	bool * m_sortascending1;
	bool * m_sortascending2;

	std::string * m_filter;
	int  * m_filtertype;
	bool * m_filterinclude;
	bool * m_filtermatch;
	af::RegExp m_filter_re;

	CtrlSortFilterMenu * m_sort_menu1;
	CtrlSortFilterMenu * m_sort_menu2;
	CtrlSortFilterMenu * m_filter_menu;
};
