#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/afnode.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "listitems.h"

class QVBoxLayout;
class QMouseEvent;

class ItemNode;
class CtrlSortFilter;

class ListNodes : public ListItems
{
   Q_OBJECT

public:

   ListNodes( QWidget* parent, const std::string & type, int RequestMsgType = 0);
   virtual ~ListNodes();

	enum e_HideShow {
		e_HideInvert     = 1<<0,
		e_HideHidden     = 1<<2,
		e_HideDone       = 1<<3,
		e_HideOffline    = 1<<4,
		e_HideError      = 1<<5
	};

	int32_t getFlagsHideShow() const { return ms_flagsHideShow; }

public slots:
	void actHideShow( int i_type);

protected:

   virtual bool init( bool createModelView = true);

   virtual ItemNode* createNewItem( af::Node *node) = 0;

   bool updateItems( af::Msg* msg);

   CtrlSortFilter * ctrl;
   void initSortFilterCtrl();

   bool sorting;
   bool sortascending;
   void sort();
   void sortMatch( const std::vector<int32_t> & i_list);
   virtual void resetSorting();

   bool setFilter( const QString & str);
   bool filtering;
   bool filterinclude;
   bool filtermatch;
   void filter();

private slots:
   void sortTypeChanged();
   void sortDirectionChanged();
   void filterChanged();
   void filterTypeChanged();
   void filterSettingsChanged();

private:
	void processHidden( ItemNode * i_item, int i_row);

private:
	static uint32_t ms_flagsHideShow;

   QRegExp filter_exp;
   QString filter_str;
};
