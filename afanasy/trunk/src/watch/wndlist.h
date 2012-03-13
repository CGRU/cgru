#pragma once

#include "../libafanasy/name_af.h"

#include "wnd.h"

class QVBoxLayout;

class ListItems;

class WndList : public Wnd
{
public:
   WndList( const QString & Name, int monType = 0);
   ~WndList();

   void setList( ListItems * list);

   void repaintItems();

private:
   int type;
   QVBoxLayout * layout;
   ListItems * itemslist;
};
