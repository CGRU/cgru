#pragma once

#include "../libafqt/name_afqt.h"

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "infoline.h"
#include "item.h"
#include "receiver.h"

#include <QtCore/QMutex>
#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QKeyEvent;
class QMouseEvent;
class QAction;
class QIcon;
class QSplitter;

class QItemSelection;
class QModelIndex;

class CtrlSortFilter;
class ButtonPanel;
class ButtonsMenu;
class ModelItems;
class Param;
class ParamsPanel;
class ViewItems;

class ListItems : public QWidget, public Receiver
{
Q_OBJECT
public:

	static const std::string & itemTypeToAf(Item::EType i_type);

	ListItems( QWidget* parent, const std::string & type = "");
	virtual ~ListItems();

	int count() const;

	void inline displayInfo(    const QString &message) { m_infoline->displayInfo(    message); }
	void inline displayWarning( const QString &message) { m_infoline->displayWarning( message); }
	void inline displayError(   const QString &message) { m_infoline->displayError(   message); }

	void revertModel();
	void itemsHeightChanged();
	void itemsHeightCalc();

	virtual bool mousePressed( QMouseEvent * event);
	virtual void keyPressEvent( QKeyEvent * i_evt);

	inline const std::string & getType() const { return m_type; }
	inline bool isTypeJobs()  const { return m_type == "jobs";  }
	inline bool isTypeUsers() const { return m_type == "users"; }

	virtual void v_itemToBeDeleted(Item * i_item);

public slots:
	void repaintItems();
	void deleteAllItems();
	void changeParam(const Param * i_param);

protected:

	void addParam(Param * i_param);
	void addParam_Num(const QString & i_name, const QString & i_label, const QString & i_tip, int i_min = -1, int i_max = -1);
	void addParam_Str(const QString & i_name, const QString & i_label, const QString & i_tip);
	void addParam_REx(const QString & i_name, const QString & i_label, const QString & i_tip);
	void addParam_Tim(const QString & i_name, const QString & i_label, const QString & i_tip);
	void addParam_Hrs(const QString & i_name, const QString & i_label, const QString & i_tip);

	void initListItems();

	void addMenuParameters(QMenu * i_menu);

	virtual void doubleClicked( Item * item);

	void getItemInfo(Item::EType i_type, const std::string & i_mode);

	inline void setParameter(Item::EType i_type, const std::string & i_name, long long i_value)
		{ setParameter(i_type, i_name, af::itos(i_value), false);}
	void setParameterRE(Item::EType i_type, const std::string & i_name, const std::string & i_value);
	void setParameter(Item::EType i_type, const std::string & i_name, const std::string & i_value, bool i_quoted = true);

	void operation(Item::EType i_type, const std::string & i_operation);

	void deleteItems(const std::vector<int32_t> & i_ids, Item::EType i_type);

	void setAllowSelection( bool allow);
	Item* getCurrentItem() const;
	int getSelectedItemsCount() const;
	const QList<Item*> getSelectedItems() const;
	const std::vector<int> getSelectedIds(Item::EType & io_type) const;
	void storeSelection();
	void reStoreSelection();

	ButtonsMenu * addButtonsMenu(const QString & i_label, const QString & i_tip);
	void resetButtonsMenu();

	ButtonPanel * addButtonPanel(
		const QString & i_label,
		const QString & i_name,
		const QString & i_description,
		const QString & i_hotkey = "",
		bool i_dblclick = false);

	void updatePanels(Item * i_item = NULL);

protected:
	//QHBoxLayout * m_hlayout;
	QSplitter   * m_splitter;
	QVBoxLayout * m_panel_l;
	QVBoxLayout * m_vlayout;

	Item * m_current_item;

	CtrlSortFilter * m_ctrl_sf;

	ParamsPanel * m_paramspanel;

	InfoLine * m_infoline;

	ViewItems * m_view;
	ModelItems * m_model;

	QWidget * m_parentWindow;

	QMutex m_mutex;

private slots:
	void currentItemChanged(const QModelIndex & current, const QModelIndex & previous);
	void selectionChanged(const QItemSelection & i_selected, const QItemSelection & i_deselected);
	void doubleClicked_slot(const QModelIndex & index);

private:
	std::string m_type;

	QList<Param*> m_params;

	ButtonsMenu * m_current_buttons_menu;

	std::vector<ButtonPanel*> m_btns;

	QList<Item*> m_stored_selection;
};
