#pragma once

#include "actionid.h"
#include "listitems.h"

#include <QtGui/QAction>
#include <QtGui/QWidget>

class QLabel;
class QHBoxLayout;

class ListItems;

class CtrlSortFilter : public QWidget
{
Q_OBJECT
public:
   CtrlSortFilter( QWidget * parent,
      int * SortType, bool * SortAscending, int * FilterType, bool * FilterInclude, bool * FilterMatch, QString * FilterString);
   ~CtrlSortFilter();

   inline void addSortType(   int type) { if(type < TLAST)   sortsarray[type] = true; }
   inline void addFilterType( int type) { if(type < TLAST) filtersarray[type] = true; }

   inline int  getSortType()      const { return   *sorttype;       }
   inline bool isSortAscending()  const { return   *sortascending;  }
   inline bool isSortDescending() const { return !(*sortascending); }

   inline const QString & getFilter()  const { return   *filter;         }
   inline int  getFilterType()         const { return   *filtertype;     }
   inline bool isFilterInclude()       const { return   *filterinclude;  }
   inline bool isFilterExclude()       const { return !(*filterinclude); }
   inline bool isFilterMatch()         const { return   *filtermatch;    }
   inline bool isFilterContain()       const { return !(*filtermatch);   }

   inline QHBoxLayout * getLayout() { return layout;}

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
      TTIMECREATION,
      TTIMELAUNCHED,
      TTIMESTARTED,
      TTIMEREGISTERED,
      TTIMEACTIVITY,
      TTIMEFINISHED,
      TTIMERUN,
      TVERSION,
      TADDRESS,
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
   void contextMenuEvent( QContextMenuEvent *event);
   void paintEvent( QPaintEvent * event);

private slots:
   void actSortType( int type);
   void actSortAscending();
   void actFilter( const QString & str);
   void actFilterInclude();
   void actFilterMacth();
   void actFilterType( int type);

private:

   bool sortsarray[TLAST];
   bool filtersarray[TLAST];

   ListItems * parernlist;

   QHBoxLayout * layout;

   QLabel * label;
   void selLabel();

   int  * sorttype;
   bool * sortascending;

   QString * filter;
   int  * filtertype;
   bool * filterinclude;
   bool * filtermatch;
};
