#include "listnodes.h"

#include "itemnode.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"

#include <QtGui/QBoxLayout>
#include <QtCore/QEvent>
#include <QtCore/QTimer>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

uint32_t ListNodes::ms_flagsHideShow = e_HideHidden;

ListNodes::ListNodes( QWidget* parent, const std::string & type, int RequestMsgType):
   ListItems( parent, type, RequestMsgType),
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
      m_model = new ModelNodes(this);
      m_view = new ViewItems( this);
      m_view->setModel( m_model);

      if( ctrl ) m_layout->addWidget( ctrl);
      m_layout->addWidget( m_view);
      m_layout->addWidget( m_infoline);
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
   QMutexLocker lock( &m_mutex);

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

   for( int i = 0; i < m_model->count(); i++)
   {
      ItemNode * itemnode = (ItemNode*)(m_model->item(i));
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
            if( oldheight != itemnode->getHeight()) m_view->emitSizeHintChanged( m_model->index(i));

				// process show/hide node if we are not going to sort all of them
				if( itemsToSort.size() == 0 )
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
      ((ModelNodes*)m_model)->sortnodes( itemsToSort, sortascending);
      setSelectedItems( selectedItems);
   }
   else if( firstChangedRow != -1 ) m_model->emit_dataChanged( firstChangedRow, lastChangedRow);

//
// close widgets with zero (non-existing) ids
   m_model->deleteZeroItems();

//
// adding new items
   bool newitemscreated = false;
   for( int j = 0; j < quantity; j++)
   {
      af::Node * node = (af::Node*)((*list)[j]);
      bool exists = false;
      for( int i = 0; i < m_model->count(); i++)
      {
         Item * item = m_model->item(i);
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
         row = ((ModelNodes*)m_model)->addNodeSorted( new_item, sortascending);
      }
      else row = ((ModelNodes*)m_model)->addNode( new_item);

		if( filtering )
			new_item->setFilterType( ctrl->getFilterType() );

		if( itemsToSort.size() == 0 )
			processHidden( new_item, row);

      if( newitemscreated == false ) newitemscreated = true;

AFINFA( "adding item \"%s\", id=%d\n", new_item->getName().toUtf8().data(), new_item->getId());
   }

	if( itemsToSort.size() )
	{
		filter();
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
   ((ModelNodes*)m_model)->sortnodes( sortascending);
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
   for( int i = 0; i < m_model->count(); i++) processHidden((ItemNode*)(m_model->item(i)), i);
}

void ListNodes::processHidden( ItemNode * i_item, int i_row)
{
	if( i_item == NULL)
	{
		AFERROR("ListNodes::filter: Item pointer is NULL.\n");
		return;
	}

	bool hidden;

	if(( filtering == false ) || ( filter_str.isEmpty() ))
	{
		hidden = false;
	}
	else
	{
		hidden = i_item->filter( filter_exp, filtermatch) != filterinclude;
	}

	if( hidden == false )
		hidden = i_item->getHidden( ms_flagsHideShow);

	if( hidden != m_view->isRowHidden( i_row))
		m_view->setRowHidden( i_row , hidden);

//if(hidden)printf("Hidding node '%s'\n", i_item->getName().toUtf8().data()); else printf("Showing node '%s'\n", i_item->getName().toUtf8().data());
}

void ListNodes::sortTypeChanged()
{
   for( int i = 0; i < m_model->count(); i++) ((ItemNode*)(m_model->item(i)))->setSortType( ctrl->getSortType());

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
	for( int i = 0; i < m_model->count(); i++)
		((ItemNode*)(m_model->item(i)))->setFilterType( ctrl->getFilterType());

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
	filter();
}

void ListNodes::sortMatch( const std::vector<int32_t> & i_list)
{
   QList<Item*> selectedItems( getSelectedItems());
   ((ModelNodes*)m_model)->sortMatch( i_list);
   setSelectedItems( selectedItems);
}
