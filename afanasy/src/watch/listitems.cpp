#include "listitems.h"

#include "../libafanasy/regexp.h"

#include "../libafqt/qenvironment.h"

#include "actionid.h"
#include "buttonpanel.h"
#include "buttonsmenu.h"
#include "ctrlsortfilter.h"
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

const std::string & ListItems::itemTypeToAf(Item::EType i_type)
{
	static const std::string branch ("branches");
	static const std::string job    ("jobs"    );
	static const std::string monitor("monitors");
	static const std::string pool   ("pools"   );
	static const std::string render ("renders" );
	static const std::string user   ("users"   );

	static const std::string invalid("invalid" );

	switch(i_type)
	{
	case Item::TBranch:
		return branch;
	case Item::TJob:
		return job;
	case Item::TMonitor:
		return monitor;
	case Item::TPool:
		return pool;
	case Item::TRender:
		return render;
	case Item::TUser:
		return user;
	case Item::TNone:
		AF_ERR << "Can't translate Item::TNone to Afanasy.";
		return invalid;
	case Item::TAny:
		AF_ERR << "Can't translate Item::TAny to Afanasy.";
		return invalid;
	default:
		AF_ERR << "Can't translate unknown Item type to Afanasy.";
	}

	return invalid;
}

ListItems::ListItems(QWidget * i_parent, const std::string & i_type):
	QWidget(i_parent),
	m_type(i_type),
	m_model(NULL),
	m_ctrl_sf(NULL),
	m_paramspanel(NULL),
	m_current_buttons_menu(NULL),
	m_parentWindow(i_parent),
	m_current_item(NULL)
{
	setAttribute ( Qt::WA_DeleteOnClose, true );

	QHBoxLayout * hlayout = new QHBoxLayout(this);

	m_panel_lelf_widget = new QWidget();
	hlayout->addWidget(m_panel_lelf_widget);
	m_panel_left_layout = new QVBoxLayout();
	m_panel_lelf_widget->setLayout(m_panel_left_layout);
	m_panel_lelf_widget->setFixedWidth(100);
	m_panel_left_layout->setAlignment(Qt::AlignTop);
	m_panel_left_layout->setContentsMargins(0, 5, 0, 5);
	m_panel_left_layout->setSpacing(5);

	m_splitter = new QSplitter();
	m_splitter->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(m_splitter);

	m_vlayout = new QVBoxLayout();
	m_vlayout->setSpacing(0);
	m_vlayout->setContentsMargins(0, 0, 0, 0);

	m_toplayout = new QHBoxLayout();
	m_vlayout->setSpacing(0);
	m_toplayout->setContentsMargins(0, 0, 0, 0);
	m_vlayout->addLayout(m_toplayout);

	QWidget * widget = new QWidget();
	m_splitter->addWidget(widget);
	widget->setLayout(m_vlayout);

	m_infoline = new InfoLine(this);

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
		m_toplayout->insertWidget(0, m_ctrl_sf);
	m_vlayout->addWidget(m_view);
	m_vlayout->addWidget(m_infoline);

	// Hide left panel if there is no buttons
	if (m_btns.size() == 0)
		m_panel_lelf_widget->setHidden(true);

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

bool ListItems::mousePressed(QMouseEvent * i_event)
{
	QModelIndex index = m_view->indexAt(i_event->pos());
	if (Item::isItemP(index.data()) == false)
		return false;

	Item * item = Item::toItemP(index.data());

	int old_height = item->getHeight();

	if (item->mousePressed(i_event->pos(), m_view->visualRect(index), i_event->buttons()))
	{
		if (item->getHeight() != old_height)
			m_view->emitSizeHintChanged(index);
		else
			m_view->update(index);
		return true;
	}

	return false;
}

void ListItems::deleteAllItems() { m_model->deleteAllItems();}
void ListItems::v_doubleClicked(Item * i_item) {}
void ListItems::revertModel()  { m_model->revert();}
void ListItems::itemsHeightChanged() { m_model->itemsHeightChanged();}
void ListItems::itemsHeightCalc() { m_model->itemsHeightCalc();}
void ListItems::repaintItems() { m_view->repaintViewport();}

void ListItems::deleteItems(const std::vector<int32_t> & i_ids, Item::EType i_type)
{
	int row = 0;
	while (row < m_model->count())
	{
		bool deleted = false;
		for (int i = 0; i < i_ids.size(); i++)
		{
			Item * item = m_model->item(row);

			if (i_type != item->getType())
				continue;

			if (i_ids[i] != item->getId())
				continue;

			m_model->delItem(row);
			deleted = true;
			break;
		}

		if (deleted)
			continue;

		row++;
	}
}
void ListItems::v_itemToBeDeleted(Item * i_item){}

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

void ListItems::storeSelection()
{
	m_stored_selection = getSelectedItems();
}

void ListItems::reStoreSelection()
{
	m_view->clearSelection();
	if (m_stored_selection.count() < 1)
		return;

	int modelcount = m_model->count();
	int lastselectedrow = -1;
	for (int i = 0; i < modelcount; i++)
	{
		if (m_stored_selection.contains(m_model->item(i)))
		{
			m_view->selectionModel()->select(m_model->index(i), QItemSelectionModel::Select);
			lastselectedrow = i;
		}
	}

	if( lastselectedrow != -1)
		m_view->selectionModel()->setCurrentIndex( m_model->index(lastselectedrow), QItemSelectionModel::Current);

	m_stored_selection.clear();
}

void ListItems::doubleClicked_slot( const QModelIndex & index )
{
	if (Item::isItemP(index.data()))
		v_doubleClicked(Item::toItemP(index.data()));
}

void ListItems::currentItemChanged( const QModelIndex & current, const QModelIndex & previous )
{
	if (m_stored_selection.count())
		return;

	Item * item = NULL;

	if (Item::isItemP(current.data()))
		item = Item::toItemP(current.data());

	if (item)
	{
		displayInfo(item->v_getSelectString());
		updatePanels(item);
	}

	m_current_item = item;
}

void ListItems::selectionChanged(const QItemSelection & i_selected, const QItemSelection & i_deselected)
{
	if (m_stored_selection.count())
		return;

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
	// Show hide panel button menus:
	for (int i = 0; i < m_btn_menus.size(); i++)
	{
		if (i_item == NULL)
		{
			m_btn_menus[i]->setHidden(false);
			m_btn_menus[i]->setActive(false);
			continue;
		}

		m_btn_menus[i]->setActive(true);

		if (m_btn_menus[i]->getType() != Item::TAny)
		{
			if (m_btn_menus[i]->getType() != i_item->getType())
				m_btn_menus[i]->setHidden(true);
			else
				m_btn_menus[i]->setHidden(false);
		}
	}

	// Show hide panel buttons:
	for (int i = 0; i < m_btns.size(); i++)
	{
		if (i_item == NULL)
		{
			m_btns[i]->setHidden(false);
			m_btns[i]->setActive(false);
			continue;
		}

		m_btns[i]->setActive(true);

		if (m_btns[i]->getType() != Item::TAny)
		{
			if (m_btns[i]->getType() != i_item->getType())
				m_btns[i]->setHidden(true);
			else
				m_btns[i]->setHidden(false);
		}
	}

	m_paramspanel->v_updatePanel(i_item);
}

void ListItems::getItemInfo(Item::EType i_type, const std::string & i_mode)
{
	Item * item = getCurrentItem();
	if (item == NULL)
		return;

	if (Item::TAny != i_type)
	{
		if (item->getType() != i_type)
			return;
	}
	else
		i_type = item->getType();

	displayInfo(QString("GET: \"%1\"").arg( afqt::stoq(i_mode)));

	std::ostringstream str;

	str << "{\"get\":{";
	str << "\"binary\":true";
	str << ",\"type\":\"" << itemTypeToAf(i_type) << "\"";
	str << ",\"ids\":[" << item->getId() << "]";
	str << ",\"mode\":\"" << i_mode << "\"";
	str << "}}";

	Watch::sendMsg( af::jsonMsg( str));
}

void ListItems::setParameter(Item::EType i_type, const std::string & i_name, const std::string & i_value)
{
	std::vector<int> ids = getSelectedIds(i_type);
	if (ids.size() == 0)
	{
		displayWarning("No items selected.");
		return;
	}

	displayInfo(QString("\"%1\" = \"%2\"").arg( afqt::stoq(i_name), afqt::stoq( i_value)));

	std::ostringstream str;

	af::jsonActionParamsStart(str, itemTypeToAf(i_type), "", ids);

	str << "\n\"" << i_name << "\":" << i_value;

	af::jsonActionParamsFinish( str);

	Watch::sendMsg( af::jsonMsg( str));
}

void ListItems::operation(Item::EType i_type, const std::string & i_operation)
{
	std::vector<int> ids = getSelectedIds(i_type);
	if (ids.size() == 0)
	{
		displayWarning("No items selected.");
		return;
	}

	std::ostringstream str;
	af::jsonActionOperation(str, itemTypeToAf(i_type), i_operation, "", ids);
	Watch::sendMsg(af::jsonMsg(str));

	displayInfo(QString("Operation: \"%1\".").arg(afqt::stoq(i_operation)));
}

const std::vector<int> ListItems::getSelectedIds(Item::EType & io_type) const
{
	std::vector<int> ids;

	QModelIndexList indexes(m_view->selectionModel()->selectedIndexes());

	Item::EType type = Item::TNone;
	for (int i = 0; i < indexes.count(); i++)
		if (Item::isItemP(indexes[i].data()))
		{
			Item * item = Item::toItemP(indexes[i].data());
			if (item->isHidden())
				continue;

			// On TAny type, we collect items that are
			// the same time, as first selected item.
			if (io_type == Item::TAny)
			{
				if (type == Item::TNone)
					type = item->getType();
				else if(type != item->getType())
					continue;
			}
			else if(item->getType() != io_type)
				continue;

			ids.push_back(item->getId());
		}

	if (io_type == Item::TAny)
		io_type = type;

	return ids;
}

ButtonsMenu * ListItems::addButtonsMenu(Item::EType i_type, const QString & i_label, const QString & i_tip)
{
	m_current_buttons_menu = new ButtonsMenu(this, i_type, i_label, i_tip);

	m_btn_menus.push_back(m_current_buttons_menu);

	m_panel_left_layout->addWidget(m_current_buttons_menu);

	return m_current_buttons_menu;
}

void ListItems::resetButtonsMenu()
{
	m_current_buttons_menu = NULL;
}

ButtonPanel * ListItems::addButtonPanel(
		Item::EType i_type,
		const QString & i_label,
		const QString & i_name,
		const QString & i_description,
		const QString & i_hotkey,
		bool i_dblclick,
		bool i_always_active)
{
	ButtonPanel * bp = new ButtonPanel(this, i_type, i_label, i_name, i_description,
			i_hotkey, i_dblclick, i_always_active, m_current_buttons_menu);

	if (m_current_buttons_menu)
		m_current_buttons_menu->addButton(bp);
	else
		m_panel_left_layout->addWidget(bp);

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

void ListItems::addParam_separator(Item::EType i_type)
{
	addParam(new Param(Param::tsep, i_type, "separator", "Separator", "This is parameters separator."));
}

void ListItems::addParam_Num(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip,
		int i_min, int i_max)
{
	addParam(new Param(Param::TNum, i_type, i_name, i_label, i_tip, i_min, i_max));
}

void ListItems::addParam_MiB(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip,
		int i_min, int i_max)
{
	addParam(new Param(Param::TMiB, i_type, i_name, i_label, i_tip, i_min, i_max));
}

void ListItems::addParam_GiB(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip,
		int i_min, int i_max)
{
	addParam(new Param(Param::TGiB, i_type, i_name, i_label, i_tip, i_min, i_max));
}

void ListItems::addParam_Str(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::TStr, i_type, i_name, i_label, i_tip));
}

void ListItems::addParam_MSI(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::TMSI, i_type, i_name, i_label, i_tip));
}

void ListItems::addParam_REx(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::TREx, i_type, i_name, i_label, i_tip));
}

void ListItems::addParam_Tim(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::Time, i_type, i_name, i_label, i_tip));
}

void ListItems::addParam_Hrs(
		Item::EType i_type,
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParam(new Param(Param::THrs, i_type, i_name, i_label, i_tip));
}

void ListItems::addMenuParameters(QMenu * i_menu)
{
	QList<Param*>::const_iterator it;
	for (it = m_params.begin(); it != m_params.end(); it++)
	{
		if ((*it)->isSeparator())
		{
			i_menu->addSeparator();
			continue;
		}

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
	if (false == i_param->getInputDialog(var, str, this))
	{
		if (false == str.isEmpty())
			displayError(str);
		return;
	}

	setParameter(i_param->itemtype, afqt::qtos(i_param->name), afqt::qtos(str));
}

