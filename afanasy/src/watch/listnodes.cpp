#include "listnodes.h"

#include "itemnode.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "monitorhost.h"
#include "viewitems.h"
#include "watch.h"
#include "wndcustomdata.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QBoxLayout>
#include <QInputDialog>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

uint32_t ListNodes::ms_flagsHideShow = e_HideHidden;

ListNodes::ListNodes( QWidget * i_parent, const std::string & i_type):
	ListItems( i_parent, i_type),
	m_ctrl_sf( NULL),
	m_subscribed( false)
{
AFINFO("ListNodes::ListNodes.\n");
}

ListNodes::~ListNodes()
{
AFINFO("ListNodes::~ListNodes.\n");

	unSubscribe();
}

bool ListNodes::init( bool createModelView)
{
	if( createModelView)
	{
		m_model = new ModelNodes(this);
		m_view = new ViewItems( this);
		m_view->setModel( m_model);

		if( m_ctrl_sf ) m_vlayout->addWidget( m_ctrl_sf);
		m_vlayout->addWidget( m_view);
		m_vlayout->addWidget( m_infoline);
	}

	if( ListItems::init( false) == false) return false;

	return true;
}

void ListNodes::initSortFilterCtrl()
{
	m_ctrl_sf->init();

	connect( m_ctrl_sf, SIGNAL( sortTypeChanged()        ), this, SLOT( sortTypeChanged()       ));
	connect( m_ctrl_sf, SIGNAL( sortDirectionChanged()   ), this, SLOT( sortDirectionChanged()  ));
	connect( m_ctrl_sf, SIGNAL( filterChanged()          ), this, SLOT( filterChanged()         ));
	connect( m_ctrl_sf, SIGNAL( filterTypeChanged()      ), this, SLOT( filterTypeChanged()     ));
	connect( m_ctrl_sf, SIGNAL( filterSettingsChanged()  ), this, SLOT( filterSettingsChanged() ));
}

void ListNodes::get() const
{
	std::string str = "\"type\":\"";
	str += getType() + "\"";
	Watch::get( str);
}

void ListNodes::get( const std::vector<int32_t> & i_ids) const
{
	if( i_ids.size() == 0 )
	{
		AFERROR("ListNodes::get: Zero length ids.")
		return;
	}

	std::string str = "\"type\":\"";
	str += getType() + "\",\"ids\":[";
	for( int i = 0; i < i_ids.size(); i++)
	{
		if( i ) str += ',';
		str += af::itos( i_ids[i]);
	}
	str += "]";

	Watch::get( str);
}

void ListNodes::subscribe( bool i_subscribe)
{
//{"action":{"user_name":"timurhai","host_name":"pc","type":"monitors","ids":[3],"operation":{"type":"watch","class":"monitors","status":"subscribe"}}}
//{"action":{"user_name":"timurhai","host_name":"pc","type":"monitors","ids":[1],"operation":{"type":"watch","class":"monitors","status":"unsubscribe"}}}
	if( m_subscribed == i_subscribe ) return;

	MonitorHost::subscribe( getType(), i_subscribe);

	m_subscribed = i_subscribe;
}

void ListNodes::v_connectionLost()
{
	unSubscribe();
	deleteAllItems();
}

void ListNodes::v_connectionEstablished() { if( isVisible()) v_showFunc(); }

void ListNodes::showEvent( QShowEvent * event)
{
	v_showFunc();
}

void ListNodes::v_showFunc()
{
	if( Watch::isConnected())
		get();
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
				int     sort_int_val1 = 0;
				int     sort_int_val2 = 0;
				QString sort_str_val1;
				QString sort_str_val2;
				if( m_ctrl_sf->isSortEnabled())
				{
					sort_int_val1 = itemnode->getSortInt1();
					sort_int_val2 = itemnode->getSortInt2();
					sort_str_val1 = itemnode->getSortStr1();
					sort_str_val2 = itemnode->getSortStr2();
				}

				// update node values
				itemnode->updateValues(  node, msg->type());

				// check for item new geometry height
				if( oldheight != itemnode->getHeight()) m_view->emitSizeHintChanged( m_model->index(i));

				// store last and first changed row
				if( firstChangedRow == -1 ) firstChangedRow = i;
				if(  lastChangedRow  <  i )  lastChangedRow = i;

				// sort node if sorting parameters changed
				if( m_ctrl_sf->isSortEnabled())
				{
					itemnode->setSortType( m_ctrl_sf->getSortType1(), m_ctrl_sf->getSortType2());
					if( sort_int_val1 != itemnode->getSortInt1()) itemsToSort << itemnode;
					if( sort_int_val2 != itemnode->getSortInt2()) itemsToSort << itemnode;
					if( sort_str_val1 != itemnode->getSortStr1()) itemsToSort << itemnode;
					if( sort_str_val2 != itemnode->getSortStr2()) itemsToSort << itemnode;
				}

				break;
			}
		}
	}

	// sort needed items, or simple emit data changed signal
	if( itemsToSort.size() > 0 )
	{
		QList<Item*> selectedItems = getSelectedItems();
		((ModelNodes*)m_model)->sortnodes( itemsToSort);
		setSelectedItems( selectedItems);
	}
	else if( firstChangedRow != -1 )
		m_model->emit_dataChanged( firstChangedRow, lastChangedRow);

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
		ItemNode* new_item = v_createNewItem( node, isSubscribed());
		if( new_item == NULL) continue;

		int row;
		if ( m_ctrl_sf->isSortEnabled())
		{
			new_item->setSortType( m_ctrl_sf->getSortType1(), m_ctrl_sf->getSortType2());
			row = ((ModelNodes*)m_model)->addNodeSorted( new_item);
		}
		else row = ((ModelNodes*)m_model)->addNode( new_item);

		if( m_ctrl_sf->isFilterEnabled())
			new_item->setFilterType( m_ctrl_sf->getFilterType() );

		if( newitemscreated == false ) newitemscreated = true;

AFINFA( "adding item \"%s\", id=%d\n", new_item->getName().toUtf8().data(), new_item->getId());
	}

	processHidden();

//   model->revert();
//   model->reset();
//printf("ListNodes::updateItems: END:\n");

	return newitemscreated;
}

void ListNodes::sort()
{
//printf("ListNodes::sort:\n");
	if( false == m_ctrl_sf->isSortEnabled())
	   return;

	QList<Item*> selectedItems( getSelectedItems());
	((ModelNodes*)m_model)->sortnodes();
	setSelectedItems( selectedItems);

	if( m_ctrl_sf->isFilterEnabled()) processHidden();
}

void ListNodes::processHidden()
{
	for( int i = 0; i < m_model->count(); i++)
	{
		ItemNode * item = (ItemNode*)(m_model->item(i));

		bool hidden = item->filter();

		if( hidden == false )
			hidden = item->getHidden( ms_flagsHideShow);

		if( hidden != m_view->isRowHidden( i))
			m_view->setRowHidden( i, hidden);

//if(hidden) printf("Hidding node '%s'\n", item->getName().toUtf8().data()); else printf("Showing node '%s'\n", item->getName().toUtf8().data());
	}
}

void ListNodes::sortTypeChanged()
{
	for( int i = 0; i < m_model->count(); i++)
		((ItemNode*)(m_model->item(i)))->setSortType( m_ctrl_sf->getSortType1(), m_ctrl_sf->getSortType2());

	if( m_ctrl_sf->isSortEnabled())
		sort();
	else
		v_resetSorting();
}

void ListNodes::v_resetSorting(){}

void ListNodes::sortDirectionChanged()
{
	if( m_ctrl_sf->isSortEnabled())
		sort();
}

void ListNodes::filterChanged()
{
	if( m_ctrl_sf->isFilterEnabled())
		processHidden();
}

void ListNodes::filterTypeChanged()
{
	for( int i = 0; i < m_model->count(); i++)
		((ItemNode*)(m_model->item(i)))->setFilterType( m_ctrl_sf->getFilterType());

	processHidden();
}

void ListNodes::filterSettingsChanged()
{
	if( m_ctrl_sf->isFilterEnabled())
		processHidden();
}

void ListNodes::actHideShow( int i_type )
{
	ms_flagsHideShow ^= i_type;
	processHidden();
}

void ListNodes::sortMatch( const std::vector<int32_t> & i_list)
{
	QList<Item*> selectedItems( getSelectedItems());
	((ModelNodes*)m_model)->sortMatch( i_list);
	setSelectedItems( selectedItems);
}

void ListNodes::actPriority()
{
	ItemNode * item = (ItemNode*)getCurrentItem();
	if( item == NULL ) return;
	int current = item->m_priority;

	int maximum = 250;
	if(( isTypeUsers()) && ( true != af::Environment::VISOR()))
		maximum = af::Environment::getPriority();

	bool ok;
	int priority = QInputDialog::getInt( this, "Change Priority", "Enter New Priority", current, 0, maximum, 1, &ok);
	if( !ok) return;

	setParameter("priority", priority);
}

void ListNodes::actAnnotate()
{
	ItemNode * item = (ItemNode*)getCurrentItem();
	if( item == NULL ) return;
	QString current = item->m_annotation;

	bool ok;
	QString text = QInputDialog::getText( this, "Annotate", "Enter Annotation", QLineEdit::Normal, current, &ok);
	if( !ok) return;

	setParameter("annotation", afqt::qtos( text));
}

void ListNodes::actAnnotate(QString text)
{
	setParameter("annotation", afqt::qtos( text));
}

void ListNodes::actCustomData()
{
	ItemNode * item = (ItemNode*)getCurrentItem();
	if( item == NULL ) return;

	WndCustomData * wnd = new WndCustomData("Custom Data", item->m_custom_data);

	connect( wnd, SIGNAL( textEdited( const QString & )), this, SLOT( customDataSet( const QString & )));
}

void ListNodes::customDataSet( const QString & text)
{
	setParameter("custom_data", af::strEscape(afqt::qtos( text)));
}
