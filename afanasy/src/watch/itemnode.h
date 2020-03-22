#pragma once

#include "item.h"
#include "listnodes.h"

#include "../libafanasy/afnode.h"

class CtrlSortFilter;
class MainWidget;

class ItemNode : public Item
{
public:
	ItemNode(ListNodes * i_list_nodes, af::Node * i_node, EType i_type, const CtrlSortFilter * i_ctrl_sf);
	virtual ~ItemNode();

	virtual const QVariant v_getToolTip() const { return m_tooltip;}

	/// Update ItemNode attributes ( copy them from given node).
	void updateValues(af::Node * i_afnode, int i_msgType);
	virtual void v_updateValues(af::Node * i_afnode, int i_msgType) = 0;

	virtual void v_setSortType(   int i_type1, int i_type2 ) = 0;
	virtual void v_setFilterType( int i_type ) = 0;

	inline void resetSorting()   { m_sort_int1 = 0; m_sort_int2 = 0; m_sort_str1.clear(); m_sort_str2.clear(); }
	inline void resetFiltering() { m_filter_str.clear(); }

	void addChild(ItemNode * i_item);

	bool compare( const ItemNode & i_other) const;
	bool filter();

	inline const QString   & getSortForce()const { return m_sort_force;}
	inline const long long & getSortInt1() const { return m_sort_int1; }
	inline const long long & getSortInt2() const { return m_sort_int2; }
	inline const QString   & getSortStr1() const { return m_sort_str1; }
	inline const QString   & getSortStr2() const { return m_sort_str2; }

	inline void setHideFlag_Hidden( bool i_value)
		{if(i_value) m_hide_flags |= ListNodes::e_HideHidden;  else m_hide_flags &= ~ListNodes::e_HideHidden;  }
	inline void setHideFlag_Done(   bool i_value)
		{if(i_value) m_hide_flags |= ListNodes::e_HideDone;    else m_hide_flags &= ~ListNodes::e_HideDone;    }
	inline void setHideFlag_Offline(bool i_value)
		{if(i_value) m_hide_flags |= ListNodes::e_HideOffline; else m_hide_flags &= ~ListNodes::e_HideOffline; }
	inline void setHideFlag_Empty(  bool i_value)
		{if(i_value) m_hide_flags |= ListNodes::e_HideEmpty;   else m_hide_flags &= ~ListNodes::e_HideEmpty;   }
	inline void setHideFlag_Error(   bool i_value)
		{if(i_value) m_hide_flags |=ListNodes::e_HideError;    else m_hide_flags &= ~ListNodes::e_HideError;   }
	inline void setHideFlag_System( bool i_value)
		{if(i_value) m_hide_flags |= ListNodes::e_HideSystem;  else m_hide_flags &= ~ListNodes::e_HideSystem;  }

	bool getHideFlags(int32_t i_hide_flags) const;

	inline const QString & getParentPath() const {return m_parent_path;}

	inline ItemNode * getParentItem() {return m_parent_item;}

	int m_priority;
	QString m_annotation;
	QString m_custom_data;

protected:
	QString m_tooltip;

	QString m_sort_force;     ///< Force sorting for hierarchy (branch, pool)
	long long m_sort_int1;    ///< For sorting by some number
	long long m_sort_int2;    ///< For sorting by some number
	QString  m_sort_str1;     ///< For sorting by some string
	QString  m_sort_str2;     ///< For sorting by some string
	std::string m_filter_str; ///< For filtering by some string

	void updateNodeValues( const af::Node * i_node);

	inline void setParentPath(const QString & i_path){m_parent_path = i_path; m_sort_force = i_path;}

	void setParentItem(ItemNode * i_item);
	virtual void v_parentItemChanged();

	ListNodes * m_list_nodes;

private:
	int32_t m_hide_flags;

	QString m_parent_path;

	ItemNode * m_parent_item;

	const CtrlSortFilter * m_ctrl_sf;

	QList<ItemNode*> m_child_list;
};
