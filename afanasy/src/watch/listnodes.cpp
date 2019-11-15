#include "listnodes.h"

#include "itemnode.h"
#include "ctrlsortfilter.h"
#include "modelnodes.h"
#include "monitorhost.h"
#include "paramspanel.h"
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
	m_subscribed( false)
{
	m_node_types.push_back(i_type);
}

ListNodes::~ListNodes()
{
	unSubscribe();
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

void ListNodes::initListNodes()
{
	m_model = new ModelNodes(this);
	initListItems();
}

void ListNodes::get() const
{
	for (int i = 0; i < m_node_types.size(); i++)
	{
		std::string str = "\"type\":\"";
		str += m_node_types[i] + "\"";
		Watch::get(str);
	}
}

void ListNodes::get(const std::vector<int32_t> & i_ids) const
{
	get(i_ids, getType());
}

void ListNodes::get(const std::vector<int32_t> & i_ids, const std::string & i_type)
{
	if (i_ids.size() == 0)
	{
		AF_ERR << "ListNodes::get: Zero length ids (type = '" << i_type << "')";
		return;
	}

	std::string str = "\"type\":\"";
	str += i_type + "\",\"ids\":[";
	for (int i = 0; i < i_ids.size(); i++)
	{
		if (i) str += ',';
		str += af::itos(i_ids[i]);
	}
	str += "]";

	Watch::get(str);
}

void ListNodes::subscribe(bool i_subscribe)
{
	if(m_subscribed == i_subscribe) return;

	for (int i = 0; i < m_node_types.size(); i++)
		MonitorHost::subscribe(m_node_types[i], i_subscribe);

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

bool ListNodes::updateItems(af::Msg * msg, Item::EType i_type)
{
	QMutexLocker lock( &m_mutex);

	af::MCAfNodes mcNodes( msg);
	std::vector<af::Af*> * afNodesList = mcNodes.getList();
	if (afNodesList->size() == 0)
		return false;

	//
	// updating exists items and setting non-existing ids to zero

	// Store nodes that was not created (not founded)
	QList<af::Node*> newAfNodes;

	// store changed rows to emit signal to update view
	int firstChangedRow = -1;
	int  lastChangedRow = -1;

	// store items need to be resorted
	QList<ItemNode*> itemsToSort;

	for (int a = 0; a < afNodesList->size(); a++)
	{
		af::Node * node = (af::Node*)((*afNodesList)[a]);

		bool found = false;

		for( int i = 0; i < m_model->count(); i++)
		{
			ItemNode * itemnode = (ItemNode*)(m_model->item(i));

//printf("ListNodes::updateItems: loonking for '%s':\n", itemnode->getName().toUtf8().data());
			if((node->getId() == itemnode->getId()) && (i_type == itemnode->getType()))
			{
//printf("ListNodes::updateItems: updating '%s':\n", itemnode->getName().toUtf8().data());
				// update lock state
				itemnode->setLock( node->isLocked());

				// store item old geometry height
				int oldheight = itemnode->getHeight();

				// strore sorting parameters
				QString sort_force_val;
				int     sort_int_val1 = 0;
				int     sort_int_val2 = 0;
				QString sort_str_val1;
				QString sort_str_val2;
				if( m_ctrl_sf->isSortEnabled())
				{
					sort_force_val= itemnode->getSortForce();
					sort_int_val1 = itemnode->getSortInt1();
					sort_int_val2 = itemnode->getSortInt2();
					sort_str_val1 = itemnode->getSortStr1();
					sort_str_val2 = itemnode->getSortStr2();
				}

				// update node values
				itemnode->v_updateValues(node, msg->type());

				// update panels if this item is current:
				if (itemnode == m_current_item)
					m_paramspanel->v_updatePanel(itemnode);

				// check for item new geometry height
				if( oldheight != itemnode->getHeight()) m_view->emitSizeHintChanged( m_model->index(i));

				// store last and first changed row
				if( firstChangedRow == -1 ) firstChangedRow = i;
				if(  lastChangedRow  <  i )  lastChangedRow = i;

				// sort node if sorting parameters changed
				if( m_ctrl_sf->isSortEnabled())
				{
					itemnode->setSortType( m_ctrl_sf->getSortType1(), m_ctrl_sf->getSortType2());
					if(sort_force_val != itemnode->getSortForce())itemsToSort << itemnode;
					if( sort_int_val1 != itemnode->getSortInt1()) itemsToSort << itemnode;
					if( sort_int_val2 != itemnode->getSortInt2()) itemsToSort << itemnode;
					if( sort_str_val1 != itemnode->getSortStr1()) itemsToSort << itemnode;
					if( sort_str_val2 != itemnode->getSortStr2()) itemsToSort << itemnode;
				}

				found = true;
				break;
			}
		}

		if (found == false)
			newAfNodes.push_back(node);
	}

	// sort needed items, or simple emit data changed signal
	if( itemsToSort.size() > 0 )
	{
		storeSelection();
		((ModelNodes*)m_model)->sortnodes( itemsToSort);
		reStoreSelection();
	}
	else if( firstChangedRow != -1 )
		m_model->emit_dataChanged( firstChangedRow, lastChangedRow);

	//
	// close widgets with zero (non-existing) ids
	m_model->deleteZeroItems();

	//
	// adding new items
	bool newitemscreated = false;
	for (int i = 0; i < newAfNodes.size(); i++)
	{
		af::Node * node = newAfNodes[i];

		ItemNode* new_item = v_createNewItemNode(node, i_type, isSubscribed());
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
	}

	processHidden();

	return newitemscreated;
}

void ListNodes::sort()
{
//printf("ListNodes::sort:\n");
	if( false == m_ctrl_sf->isSortEnabled())
	   return;

	storeSelection();
	((ModelNodes*)m_model)->sortnodes();
	reStoreSelection();

	if( m_ctrl_sf->isFilterEnabled()) processHidden();
}

void ListNodes::processHidden()
{
	for (int i = 0; i < m_model->count(); i++)
	{
		ItemNode * item = (ItemNode*)(m_model->item(i));

		bool hidden = item->filter();

		if (false == hidden)
			hidden = item->getHiddenFlags(ms_flagsHideShow);

		item->setHidded(hidden);

		if (hidden != m_view->isRowHidden(i))
			m_view->setRowHidden(i, hidden);
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
	storeSelection();
	((ModelNodes*)m_model)->sortMatch( i_list);
	reStoreSelection();
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
