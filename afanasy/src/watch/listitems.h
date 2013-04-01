#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "infoline.h"
#include "reciever.h"

#include <QtCore/QMutex>
#include <QtGui/QWidget>

class QVBoxLayout;
class QMouseEvent;

class QModelIndex;

class Item;
class ModelItems;
class ViewItems;

class ListItems : public QWidget, public Reciever
{
Q_OBJECT
public:

	ListItems( QWidget* parent, const std::string & type = "", int RequestMsgType = 0);
	virtual ~ListItems();

	int count() const;

	void inline displayInfo(    const QString &message) { m_infoline->displayInfo(    message); }
	void inline displayWarning( const QString &message) { m_infoline->displayWarning( message); }
	void inline displayError(   const QString &message) { m_infoline->displayError(   message); }

	void revertModel();
	void itemsHeightCahnged();

	virtual bool mousePressed( QMouseEvent * event);

public slots:
	void repaintItems();
	void deleteAllItems();

protected:

	virtual bool init( bool createModelView = true);

	virtual void doubleClicked( Item * item);

	void action( af::MCGeneral & mcgeneral, int type);

	inline void setParameter( const std::string & i_name, long long i_value) const
		{ setParameter( i_name, af::itos( i_value), false);}
	void setParameter( const std::string & i_name, const std::string & i_value, bool i_quoted = true) const;

	void deleteItems( af::MCGeneral & ids);

	void setAllowSelection( bool allow);
	Item* getCurrentItem() const;
	int getSelectedItemsCount() const;
	const QList<Item*> getSelectedItems() const;
	void setSelectedItems( const QList<Item*> & items, bool resetSelection = true);

	virtual void showEvent(       QShowEvent  * event );
	virtual void hideEvent(       QHideEvent  * event );

	virtual void v_shownFunc();
	virtual void v_subscribe();
	virtual void v_unSubscribe();

	inline bool isSubscribed() const { return m_subscribed;}

	virtual void v_connectionLost();
	virtual void v_connectionEstablished();

protected:
	QVBoxLayout * m_layout;
	InfoLine * m_infoline;

	ViewItems * m_view;
	ModelItems * m_model;

	QWidget * m_parentWindow;

	QList<int> m_eventsOnOff;
	QList<int> m_eventsShowHide;

	QMutex m_mutex;

private slots:
	void currentItemChanged( const QModelIndex & current, const QModelIndex & previous );
	void doubleClicked_slot( const QModelIndex & index );

private:
	const std::vector<int> getSelectedIds() const;

private:
	std::string m_type;
	int  m_requestmsgtype;
	bool m_subscribed;
	bool m_subscribeFirstTime;
};
