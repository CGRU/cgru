#pragma once

#include "../libafanasy/afnode.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "item.h"
#include "listitems.h"

class QVBoxLayout;
class QMouseEvent;

class ItemNode;

class ListNodes : public ListItems
{
	Q_OBJECT

  public:
	ListNodes(QWidget *i_parent, const std::string &i_type);
	virtual ~ListNodes();

	enum e_HideFlags
	{
		e_HideInvert = 1 << 0,
		e_HideHidden = 1 << 2,
		e_HideDone = 1 << 3,
		e_HideOffline = 1 << 4,
		e_HideEmpty = 1 << 5,
		e_HideError = 1 << 6,
		e_HideSystem = 1 << 7,
		e_HidePools = 1 << 8,
	};

	int32_t getFlagsHideShow() const { return m_hide_flags; }

	virtual void v_connectionEstablished();

	/// Store nodes that can have childs for a quick access via map
	void hrStoreParent(ItemNode *i_item);

	void hrParentChanged(ItemNode *i_item);

	void processHidden();

  public slots:
	void actHideShow(int i_type);

  protected:
	void initListNodes();

	virtual void showEvent(QShowEvent *event);

	virtual void v_showFunc();

	void subscribe(bool i_subscribe = true);
	inline void unSubscribe() { subscribe(false); }
	inline bool isSubscribed() const { return m_subscribed; }

	virtual void v_connectionLost();

	virtual ItemNode *v_createNewItemNode(af::Node *i_afnode, Item::EType i_type, bool i_notify) = 0;

	void get() const;
	void get(const std::string &i_type) const;
	void get(const std::vector<int32_t> &i_ids) const;
	static void get(const std::vector<int32_t> &i_ids, const std::string &i_type);

	bool updateItems(af::Msg *msg, Item::EType i_type);

	void initSortFilterCtrl();

	void sort();
	void sortMatch(const std::vector<int32_t> &i_list);

	/// Needed for jobs, to get user jobs list from server
	virtual void v_resetSorting();

	// Nedded to store hide flags.
	// Each descendant class should store flags in own variable.
	virtual void v_hideChanged();

  protected:
	std::vector<std::string> m_node_types;

	uint32_t m_hide_flags;

  private slots:
	void actAnnotate();
	void actAnnotate(QString text);
	void actPriority();
	void actCustomData();

	void customDataSet(const QString &text);

	void sortTypeChanged();
	void sortDirectionChanged();
	void filterChanged();
	void filterTypeChanged();
	void filterSettingsChanged();

  private:
  private:
	bool m_subscribed;

	QMap<QString, ItemNode *> m_hr_parents_map;
};
