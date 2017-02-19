#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/afnode.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "listitems.h"

class QVBoxLayout;
class QMouseEvent;

class ItemNode;
class CtrlSortFilter;

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

	virtual void showEvent( QShowEvent  * event );

	virtual void v_showFunc();

	void subscribe( bool i_subscribe = true);
	inline void unSubscribe() { subscribe( false);}
	inline bool isSubscribed() const { return m_subscribed;}

	virtual void v_connectionLost();

	virtual bool init( bool createModelView = true);

	virtual ItemNode * v_createNewItem( af::Node * i_node, bool i_subscibed) = 0;

	void get() const;
	void get( const std::vector<int32_t> & i_ids) const;

	bool updateItems( af::Msg* msg);

	CtrlSortFilter * m_ctrl_sf;
	void initSortFilterCtrl();

	void sort();
	void sortMatch( const std::vector<int32_t> & i_list);

	/// Needed for jobs, to get user jobs list from server
	virtual void v_resetSorting();

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
