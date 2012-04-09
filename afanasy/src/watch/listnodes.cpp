#include "listnodes.h"

#include <QtGui/QBoxLayout>
#include <QtCore/QEvent>
#include <QtCore/QTimer>

#include "itemnode.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

uint32_t ListNodes::ms_flagsHideShow = e_HideHidden;

ListNodes::ListNodes( QWidget* parent, int RequestMsgType):
   ListItems( parent, RequestMsgType),
   ctrl( NULL),
   sorting( false),
   sortascending( false),
   filtering( false),
   filterinclude( true)
{
AFINFO("ListNodes::ListNodes.\n");
}

ListNodes::~ListNodes()
{
AFINFO("ListNodes::~ListNodes.\n");
}

bool ListNodes::init( bool createModelView)
{
   if( createModelView)
   {
      model = new ModelNodes(this);
      view = new ViewItems( this);
      view->setModel( model);

      if( ctrl ) layout->addWidget( ctrl);
      layout->addWidget( view);
      layout->addWidget( infoline);
   }

   if( ListItems::init( false) == false) return false;

   return true;
}

void ListNodes::initSortFilterCtrl()
{
   sorting        = ctrl->getSortType() != CtrlSortFilter::TNONE;
   sortascending  = ctrl->isSortAscending();
   filtering      = ctrl->getFilterType() != CtrlSortFilter::TNONE;
   filterinclude  = ctrl->isFilterInclude();
   filtermatch    = ctrl->isFilterMatch();
   setFilter(       ctrl->getFilter());

   connect( ctrl, SIGNAL( sortTypeChanged()        ), this, SLOT( sortTypeChanged()       ));
   connect( ctrl, SIGNAL( sortDirectionChanged()   ), this, SLOT( sortDirectionChanged()  ));
   connect( ctrl, SIGNAL( filterChanged()          ), this, SLOT( filterChanged()         ));
   connect( ctrl, SIGNAL( filterTypeChanged()      ), this, SLOT( filterTypeChanged()     ));
   connect( ctrl, SIGNAL( filterSettingsChanged()  ), this, SLOT( filterSettingsChanged() ));
}

bool ListNodes::updateItems( af::Msg * msg)
{
   QMutexLocker lock( &mutex);

   af::MCAfNodes mcNodes( msg);
   std::vector<af::Af*> * list = mcNodes.getList();

   int quantity = int( list->size());
   if( quantity == 0) return false;

//printf("ListNodes::updateItems: message list->size() = %d:\n", quantity);

//
// updating exists items and setting non-existing ids to zero

   // store changed rows to emit signal to update view
   int firstChangedRow = -1;
   int  lastChangedRow = -1;

   // store items need to be resorted
   QList<ItemNode*> itemsToSort;

   for( int i = 0; i < model->count(); i++)
   {
      ItemNode * itemnode = (ItemNode*)(model->item(i));
//printf("ListNodes::updateItems: loonking for '%s':\n", itemnode->getName().toUtf8().data());
      for( int j = 0; j < quantity; j++)
      {
         af::Node * node = (af::Node*)((*list)[j]);
         if( node->getId() == itemnode->getId())
         {
//printf("ListNodes::updateItems: updating '%s':\n", itemnode->getName().toUtf8().data());
            // update lock state
            itemnode->setLock( node->isLocked());

            // store item old geometry height
            int oldheight = itemnode->getHeight();

            // strore sorting parameters
            QString  sort_str_val;
            int      sort_int_val = 0;
            if( sorting )
            {
               sort_int_val = itemnode->getSortInt();
               sort_str_val = itemnode->getSortStr();
            }

            // update node values
            itemnode->updateValues(  node, msg->type());

            // check for item new geometry height
            if( oldheight != itemnode->getHeight()) view->emitSizeHintChanged( model->index(i));

            // filter node
            if( filtering ) filter( itemnode, i);

				// process show/hide
				processHidden( itemnode, i);

            // store last and first changed row
            if( firstChangedRow == -1 ) firstChangedRow = i;
            if(  lastChangedRow  <  i )  lastChangedRow = i;

            // sort node if sorting parameters changed
            if( sorting )
            {
               itemnode->setSortType( ctrl->getSortType());
               if( sort_int_val != itemnode->getSortInt()) itemsToSort << itemnode;
               if( sort_str_val != itemnode->getSortStr()) itemsToSort << itemnode;
            }

            break;
         }
      }
   }

   // sort needed items, or simple emit data changed signal
   if( itemsToSort.size() > 0 )
   {
      QList<Item*> selectedItems = getSelectedItems();
      ((ModelNodes*)model)->sortnodes( itemsToSort, sortascending);
      setSelectedItems( selectedItems);
   }
   else if( firstChangedRow != -1 ) model->emit_dataChanged( firstChangedRow, lastChangedRow);

//
// close widgets with zero (non-existing) ids
   model->deleteZeroItems();

//
// adding new items
   bool newitemscreated = false;
   for( int j = 0; j < quantity; j++)
   {
      af::Node * node = (af::Node*)((*list)[j]);
      bool exists = false;
      for( int i = 0; i < model->count(); i++)
      {
         Item * item = model->item(i);
         int cur_id = item->getId();
         if( node->getId() == cur_id)
         {
            exists = true;
            break;
         }
      }
      if( exists) continue;
      ItemNode* new_item = createNewItem( node);
      if( new_item == NULL) continue;

      int row;
      if ( sorting )
      {
         new_item->setSortType( ctrl->getSortType());
         row = ((ModelNodes*)model)->addNodeSorted( new_item, sortascending);
      }
      else row = ((ModelNodes*)model)->addNode( new_item);


      if( filtering )
      {
         new_item->setFilterType( ctrl->getFilterType() );
         filter( new_item, row);
      }

		processHidden( new_item, row);

      if( newitemscreated == false ) newitemscreated = true;

AFINFA( "adding item \"%s\", id=%d\n", new_item->getName().toUtf8().data(), new_item->getId());
   }

//   model->revert();
//   model->reset();
//printf("ListNodes::updateItems: END:\n");

   return newitemscreated;
}

void ListNodes::sort()
{
//printf("ListNodes::sort:\n");
   if( sorting == false ) return;
   QList<Item*> selectedItems( getSelectedItems());
   ((ModelNodes*)model)->sortnodes( sortascending);
   setSelectedItems( selectedItems);

	if( filtering ) filter();
}

bool ListNodes::setFilter( const QString & str)
{
   filter_exp.setPattern( str);
   if( filter_exp.isValid() == false)
   {
      AFERRAR("ListNodes::setFilter: Invalid pattern: \"%s\" - %s",
         str.toUtf8().data(), filter_exp.errorString().toUtf8().data());
      if( filter_str.isEmpty() == false) filter_exp.setPattern( filter_str);
      return false;
   }
   filter_str = str;
   return true;
}

void ListNodes::filter()
{
   for( int i = 0; i < model->count(); i++) filter((ItemNode*)(model->item(i)), i);
}

void ListNodes::filter( ItemNode * item, int row)
{
   if( item == NULL)
   {
      AFERROR("ListNodes::filter: Item pointer is NULL.\n");
      return;
   }
   if((filtering == false) || (filter_str.isEmpty()))
      view->setRowHidden( row , false);
   else
      view->setRowHidden( row , item->filter( filter_exp, filtermatch) != filterinclude);
}

void ListNodes::sortTypeChanged()
{
   for( int i = 0; i < model->count(); i++) ((ItemNode*)(model->item(i)))->setSortType( ctrl->getSortType());

   if((ctrl->getSortType() == CtrlSortFilter::TNONE) || (ctrl->getSortType() >= CtrlSortFilter::TLAST))
   {
      sorting = false;
      resetSorting();
   }
   else
   {
      sorting = true;
      sort();
   }
}

void ListNodes::resetSorting(){}

void ListNodes::sortDirectionChanged()
{
   sortascending = ctrl->isSortAscending();
   if( sorting) sort();
}

void ListNodes::filterChanged()
{
   if( setFilter( ctrl->getFilter()))
   {
      if( filtering) filter();
   }
}

void ListNodes::filterTypeChanged()
{
	for( int i = 0; i < model->count(); i++)
		((ItemNode*)(model->item(i)))->setFilterType( ctrl->getFilterType());

   if((ctrl->getFilterType() == CtrlSortFilter::TNONE) || (ctrl->getFilterType() >= CtrlSortFilter::TLAST))
   {
      filtering = false;
   }
   else
   {
      filtering = true;
   }
   filter();
}

void ListNodes::filterSettingsChanged()
{
   filterinclude = ctrl->isFilterInclude();
   filtermatch   = ctrl->isFilterMatch();
   if( filtering) filter();
}

void ListNodes::actHideShow( int i_type )
{
	ms_flagsHideShow ^= i_type;
	processHidden();
}

void ListNodes::processHidden()
{
	for( int i = 0; i < model->count(); i++)
		processHidden( (ItemNode*)(model->item(i)), i);
}

void ListNodes::processHidden( ItemNode * i_node, int i_row)
{
	view->setRowHidden( i_row, i_node->getHidden( ms_flagsHideShow));
}

void ListNodes::sortMatch( const std::vector<int32_t> & i_list)
{
   QList<Item*> selectedItems( getSelectedItems());
   ((ModelNodes*)model)->sortMatch( i_list);
   setSelectedItems( selectedItems);
}
