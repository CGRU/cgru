#include "listitems.h"

#include "../libafanasy/regexp.h"

#include "../libafqt/qenvironment.h"

#include "actionid.h"
#include "buttonpanel.h"
#include "buttonsmenu.h"
#include "ctrlsortfilter.h"
#include "item.h"
#include "modelitems.h"
#include "param.h"
#include "paramspanel.h"
#include "viewitems.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>
#include <QBoxLayout>
#include <QLabel>
#include <QSplitter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ListItems::ListItems( QWidget* parent, const std::string & type):
	QWidget( parent),
	m_type( type),
	m_model(NULL),
	m_ctrl_sf(NULL),
	m_paramspanel(NULL),
	m_current_buttons_menu(NULL),
	m_parentWindow(parent),
	m_current_item(NULL)
{
	setAttribute ( Qt::WA_DeleteOnClose, true );

	QHBoxLayout * hlayout = new QHBoxLayout(this);

	QWidget * panel_l_widget = new QWidget();
	hlayout->addWidget(panel_l_widget);
	m_panel_l = new QVBoxLayout();
	panel_l_widget->setLayout(m_panel_l);
	panel_l_widget->setFixedWidth(100);
	m_panel_l->setAlignment(Qt::AlignTop);
	m_panel_l->setContentsMargins(0, 5, 0, 5);
	m_panel_l->setSpacing(5);

	m_splitter = new QSplitter();
	hlayout->addWidget(m_splitter);
	m_vlayout = new QVBoxLayout();
	QWidget * widget = new QWidget();
	m_splitter->addWidget(widget);
	widget->setLayout(m_vlayout);

	m_vlayout->setSpacing(0);
	m_splitter->setContentsMargins(0, 0, 0, 0);
	m_vlayout->setContentsMargins(0, 0, 0, 0);
	m_infoline = new InfoLine(this);

//	if( m_parentWindow != (QWidget*)(Watch::getDialog())) setFocusPolicy(Qt::StrongFocus);
	setFocusPolicy(Qt::StrongFocus);
}

void ListItems::initListItems()
{
	// Descendant classes may create own panel:
	if (NULL == m_paramspanel)
		m_paramspanel = new ParamsPanel();
	m_splitter->addWidget(m_paramspanel);
	m_paramspanel->initPanel(m_params, m_splitter, afqt::stoq(m_type));
	connect(m_paramspanel, SIGNAL(sig_changeParam(const Param *)), this, SLOT(changeParam(const Param *)));

	// ListNodes creates: m_model = new ModelNodes
	if (NULL == m_model)
		m_model = new ModelItems(this);
	m_view = new ViewItems(this);
	m_view->setModel(m_model);

	// Various ListNodes (Jobs, Users, ...) create sorting filtering control.
	// ListTasks does not.
	// And it should be properly inserted.
	if (m_ctrl_sf)
		m_vlayout->addWidget(m_ctrl_sf);
	m_vlayout->addWidget(m_view);
	m_vlayout->addWidget(m_infoline);

	connect(m_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(doubleClicked_slot(const QModelIndex &)));

	connect(m_view->selectionModel(), SIGNAL(    currentChanged( const    QModelIndex &, const    QModelIndex &)),
	                            this,   SLOT(currentItemChanged( const    QModelIndex &, const    QModelIndex &)));
	connect(m_view->selectionModel(), SIGNAL(  selectionChanged( const QItemSelection &, const QItemSelection &)),
	                            this,   SLOT(  selectionChanged( const QItemSelection &, const QItemSelection &)));
}

ListItems::~ListItems()
{
	// We can`t store state just in m_paramspanel dtor,
	// as storeState() uses m_splitter, that can be destroyed first.
	m_paramspanel->storeState();

	QList<Param*>::iterator it;
	for (it = m_params.begin(); it != m_params.end(); it++)
		delete (*it);
}

int ListItems::count() const { return m_model->count();}

bool ListItems::mousePressed( QMouseEvent * event) { return false;}

void ListItems::deleteAllItems() { m_model->deleteAllItems();}
void ListItems::doubleClicked( Item * item) {}
void ListItems::revertModel()  { m_model->revert();}
void ListItems::itemsHeightChanged() { m_model->itemsHeightChanged();}
void ListItems::itemsHeightCalc() { m_model->itemsHeightCalc();}
void ListItems::repaintItems() { m_view->repaintViewport();}

void ListItems::deleteItems( const std::vector<int32_t> & i_ids)
{
	int row = 0;
	while( row < m_model->count())
	{
		bool deleted = false;
		for( int i = 0; i < i_ids.size(); i++)
		{
			if( i_ids[i] == m_model->item(row)->getId())
			{
				m_model->delItem( row);
				deleted = true;
				break;
			}
		}

		if( deleted )
			continue;

		row++;
	}
}
void ListItems::setAllowSelection( bool allow)
{
	if( allow ) m_view->setSelectionMode( QAbstractItemView::ExtendedSelection  );
	else        m_view->setSelectionMode( QAbstractItemView::NoSelection        );
}

Item * ListItems::getCurrentItem() const
{
	QModelIndex index( m_view->selectionModel()->currentIndex());
	if( index.isValid())
		if( Item::isItemP( index.data()))
			return Item::toItemP( index.data());

	QList<Item*> items = getSelectedItems();
	if( items.size())
		return items[0];

	return NULL;
}

int ListItems::getSelectedItemsCount() const
{
	return m_view->selectionModel()->selectedIndexes().size();
}

const QList<Item*> ListItems::getSelectedItems() const
{
	QList<Item*> items;

	QModelIndexList indexes( m_view->selectionModel()->selectedIndexes());
	for( int i = 0; i < indexes.count(); i++)
		if( Item::isItemP( indexes[i].data()))
			items << Item::toItemP( indexes[i].data());

	return items;
}

void ListItems::setSelectedItems( const QList<Item*> & items, bool resetSelection)
{
	if( resetSelection ) m_view->clearSelection();
	if( items.count() < 1 ) return;
	int modelcount = m_model->count();
	int lastselectedrow = -1;
	for( int i = 0; i < modelcount; i++)
	{
		if( items.contains( m_model->item(i)))
		{
			m_view->selectionModel()->select( m_model->index(i), QItemSelectionModel::Select);
			lastselectedrow = i;
		}
	}
	if( lastselectedrow != -1)
		m_view->selectionModel()->setCurrentIndex( m_model->index(lastselectedrow), QItemSelectionModel::Current);
}

void ListItems::doubleClicked_slot( const QModelIndex & index )
{
	if( Item::isItemP( index.data()))
		doubleClicked( Item::toItemP( index.data()));
}

void ListItems::currentItemChanged( const QModelIndex & current, const QModelIndex & previous )
{
	Item * item = NULL;

	if (Item::isItemP(current.data()))
		item = Item::toItemP(current.data());

	if (item)
	{
		displayInfo(item->getSelectString());
		updatePanels(item);
	}

	m_current_item = item;
}

void ListItems::selectionChanged(const QItemSelection & i_selected, const QItemSelection & i_deselected)
{
	if (m_view->selectionModel()->selectedIndexes().size() == 0)
	{
		// Everything was deselected:
		m_current_item = NULL;
		m_infoline->clear();
		updatePanels();
		return;
	}

	if (NULL == m_current_item)
	{
		// This can be if the same one node was deselected, than selected again.
		// In such case current item was not changed.
		QModelIndexList indexes = i_selected.indexes();
		if ((indexes.size()) && (Item::isItemP(indexes.last().data())))
		{
			m_current_item = Item::toItemP(indexes.last().data());
			updatePanels(m_current_item);
		}
	}
}

void ListItems::updatePanels(Item * i_item)
{
	m_paramspanel->v_updatePanel(i_item);
}

void ListItems::getItemInfo( const std::string & i_mode)
{
	Item * item = getCurrentItem();
	if( item == NULL ) return;

//{"get":{"type":"renders","ids":[1],"mode":"log"}}

	displayInfo(QString("GET: \"%1\"").arg( afqt::stoq(i_mode)));

	std::ostringstream str;

	str << "{\"get\":{";
	str << "\"binary\":true";
	str << ",\"type\":\"" << m_type << "\"";
	str << ",\"ids\":[" << item->getId() << "]";
	str << ",\"mode\":\"" << i_mode << "\"";
	str << "}}";

	Watch::sendMsg( af::jsonMsg( str));
}

void ListItems::setParameterRE( const std::string & i_name, const std::string & i_value)
{
	std::string err;

	if( af::RegExp::Validate( i_value, &err))
		setParameter( i_name, i_value, true);
	else
		displayError( afqt::stoq(err));
}

void ListItems::setParameter( const std::string & i_name, const std::string & i_value, bool i_quoted)
{
	if( getSelectedItemsCount() == 0 )
	{
		displayWarning("No items selected.");
		return;
	}

	displayInfo(QString("\"%1\" = \"%2\"").arg( afqt::stoq(i_name), afqt::stoq( i_value)));

	std::ostringstream str;

	af::jsonActionParamsStart( str, m_type, "", getSelectedIds());

	str << "\n\"" << i_name << "\":";
	if (i_quoted)
		str << "\"" << af::strEscape(i_value) << "\"";
	else
		str << i_value;

	af::jsonActionParamsFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

void ListItems::operation( const std::string & i_operation)
{
	std::ostringstream str;
	std::vector<int> ids = getSelectedIds();
	if( ids.size() == 0 )
	{
		displayWarning("No items selected.");
		return;
	}
	af::jsonActionOperation( str, m_type, i_operation, "", ids);
	Watch::sendMsg( af::jsonMsg( str));

	displayInfo(QString("Operation: \"%1\".").arg( afqt::stoq( i_operation)));
}

const std::vector<int> ListItems::getSelectedIds() const
{
	std::vector<int> ids;
	QModelIndexList indexes(m_view->selectionModel()->selectedIndexes());
	for (int i = 0; i < indexes.count(); i++)
		if (Item::isItemP(indexes[i].data()))
		{
			Item * item = Item::toItemP(indexes[i].data());
			if (false == item->isHidden())
				ids.push_back(item->getId());
		}
	return ids;
}

ButtonsMenu * ListItems::addButtonsMenu(const QString & i_label, const QString & i_tip)
{
	m_current_buttons_menu = new ButtonsMenu(this, i_label, i_tip);

	m_panel_l->addWidget(m_current_buttons_menu);

	return m_current_buttons_menu;
}

void ListItems::resetButtonsMenu()
{
	m_current_buttons_menu = NULL;
}

ButtonPanel * ListItems::addButtonPanel(
		const QString & i_label,
		const QString & i_name,
		const QString & i_description,
		const QString & i_hotkey,
		bool i_dblclick)
{
	ButtonPanel * bp = new ButtonPanel(this, i_label, i_name, i_description, i_hotkey, i_dblclick, m_current_buttons_menu);

	if (m_current_buttons_menu)
		m_current_buttons_menu->addButton(bp);
	else
		m_panel_l->addWidget(bp);

	m_btns.push_back(bp);

	return bp;
}

void ListItems::keyPressEvent( QKeyEvent * i_evt)
{
	if( i_evt->modifiers() & ( Qt::ControlModifier | Qt::AltModifier ))
		return;

	const QString str( i_evt->text());
	if( str.isNull() || str.isEmpty() )
		return;
//printf("ListItems::keyPressEvent: '%s'\n", str.toUtf8().data());

	if( ButtonPanel::setHotkey( str))
		return;

	for( int i = 0; i < m_btns.size(); i++ )
		m_btns[i]->keyPressed( str);
}

void ListItems::addParam(Param * i_param)
{
	m_params.append(i_param);
}

void ListItems::addParam_Int(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip,
		int i_min, int i_max)
{
	addParam(new Param(Param::TInt, i_name, i_label, i_tip, i_min, i_max));
}

void ListItems::addParam_Str(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::TStr, i_name, i_label, i_tip));
}

void ListItems::addParam_REx(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::TREx, i_name, i_label, i_tip));
}

void ListItems::addParam_Tim(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::Time, i_name, i_label, i_tip));
}

void ListItems::addMenuParameters(QMenu * i_menu)
{
	QList<Param*>::const_iterator it;
	for (it = m_params.begin(); it != m_params.end(); it++)
	{
		ActionParam * action = new ActionParam(*it);
		connect(action, SIGNAL(triggeredParam(const Param *)), this, SLOT(changeParam(const Param *)));
		i_menu->addAction(action);
	}
}

void ListItems::changeParam(const Param * i_param)
{
	Item * item = getCurrentItem();
	if (NULL == item)
		return;

	if (false == item->hasParam(i_param->name))
	{
		AF_ERR << "No such parameter: " << i_param->name.toUtf8().data();
		return;
	}

	QVariant var = item->getParamVar(i_param->name);

	QString str;
	if (false == i_param->getInputDialog(var, str))
	{
		if (false == str.isEmpty())
			displayError(str);
		return;
	}

	setParameter(afqt::qtos(i_param->name), afqt::qtos(str), false);
}

