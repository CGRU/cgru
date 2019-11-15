#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/afnode.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "item.h"
#include "listitems.h"

class QVBoxLayout;
class QMouseEvent;

class ItemNode;

class ListNodes : public ListItems
{
	Q_OBJECT

public:

	ListNodes( QWidget * i_parent, const std::string & i_type);
	virtual ~ListNodes();

	enum e_HideShow {
		e_HideInvert     = 1<<0,
		e_HideHidden     = 1<<2,
		e_HideDone       = 1<<3,
		e_HideOffline    = 1<<4,
		e_HideError      = 1<<5
	};

	int32_t getFlagsHideShow() const { return ms_flagsHideShow; }

	virtual void v_connectionEstablished();

public slots:
	void actHideShow( int i_type);

protected:

	void initListNodes();

	virtual void showEvent( QShowEvent  * event );

	virtual void v_showFunc();

	void subscribe( bool i_subscribe = true);
	inline void unSubscribe() { subscribe( false);}
	inline bool isSubscribed() const { return m_subscribed;}

	virtual void v_connectionLost();

	virtual ItemNode * v_createNewItemNode(af::Node * i_afnode, Item::EType i_type, bool i_notify) = 0;

	void get() const;
	void get(const std::vector<int32_t> & i_ids) const;
	static void get(const std::vector<int32_t> & i_ids, const std::string & i_type);

	bool updateItems(af::Msg* msg, Item::EType i_type);

	void initSortFilterCtrl();

	void sort();
	void sortMatch( const std::vector<int32_t> & i_list);

	/// Needed for jobs, to get user jobs list from server
	virtual void v_resetSorting();

protected:
	std::vector<std::string> m_node_types;

private slots:
	void actAnnotate();
	void actAnnotate(QString text);
	void actPriority();
	void actCustomData();

	void customDataSet( const QString & text);

	void sortTypeChanged();
	void sortDirectionChanged();
	void filterChanged();
	void filterTypeChanged();
	void filterSettingsChanged();

private:
	void processHidden();

	static uint32_t ms_flagsHideShow;

private:
	bool m_subscribed;
};
