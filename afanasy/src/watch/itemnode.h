#pragma once

#include "item.h"
#include "listnodes.h"

#include "../libafanasy/afnode.h"

class CtrlSortFilter;
class MainWidget;

class ItemNode : public Item
{
public:
	ItemNode( af::Node * i_node, const CtrlSortFilter * i_ctrl_sf);
	virtual ~ItemNode();

	virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

	virtual inline const QVariant getToolTip() const { return m_tooltip;}

	/// Update ItemNode attributes ( copy them from given node).
	virtual void updateValues( af::Node *node, int type) = 0;

	virtual void setSortType(   int i_type1, int i_type2 ) = 0;
	virtual void setFilterType( int i_type ) = 0;

	inline void resetSorting()   { m_sort_int1 = 0; m_sort_int2 = 0; m_sort_str1.clear(); m_sort_str2.clear(); }
	inline void resetFiltering() { m_filter_str.clear(); }

	bool compare( const ItemNode & i_other) const;
	bool filter();

	inline const long long & getSortInt1() const { return m_sort_int1; }
	inline const long long & getSortInt2() const { return m_sort_int2; }
	inline const QString   & getSortStr1() const { return m_sort_str1; }
	inline const QString   & getSortStr2() const { return m_sort_str2; }

	inline void setHidden(  bool i_value)
		{ if( i_value ) m_flagshidden |= ListNodes::e_HideHidden;  else m_flagshidden &= ~ListNodes::e_HideHidden;  }
	inline void setDone(    bool i_value)
		{ if( i_value ) m_flagshidden |= ListNodes::e_HideDone;    else m_flagshidden &= ~ListNodes::e_HideDone;    }
	inline void setOffline( bool i_value)
		{ if( i_value ) m_flagshidden |= ListNodes::e_HideOffline; else m_flagshidden &= ~ListNodes::e_HideOffline; }
	inline void setError(   bool i_value)
		{ if( i_value ) m_flagshidden |= ListNodes::e_HideError;   else m_flagshidden &= ~ListNodes::e_HideError;   }

	bool getHidden( int32_t i_flags) const;

	int m_priority;
	QString m_annotation;
	QString m_custom_data;

protected:
	QString m_tooltip;

	long long m_sort_int1;    ///< For sorting by some number
	long long m_sort_int2;    ///< For sorting by some number
	QString  m_sort_str1;     ///< For sorting by some string
	QString  m_sort_str2;     ///< For sorting by some string
	std::string m_filter_str; ///< For filtering by some string

	void updateNodeValues( const af::Node * i_node);

private:
	int32_t m_flagshidden;

	const CtrlSortFilter * m_ctrl_sf;
};
