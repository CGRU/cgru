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

   ListNodes( QWidget* parent, int RequestMsgType = 0);
   virtual ~ListNodes();

protected:

   virtual bool init( bool createModelView = true);

   virtual ItemNode* createNewItem( af::Node *node) = 0;

   bool updateItems( af::Msg* msg);

   CtrlSortFilter * ctrl;
   void initSortFilterCtrl();

   bool sorting;
   bool sortascending;
   void sort();
   void sortMatch( const std::vector<int32_t> * list);

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
   void filter( ItemNode * item, int row);

   QRegExp filter_exp;
   QString filter_str;
};
