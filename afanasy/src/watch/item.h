#pragma once

#include "../libafanasy/afnode.h"

#include <QItemDelegate>
#include <QStyleOption>
#include <QMenu>

class MainWidget;

class Item
{
public:
	Item(const QString &itemname, int itemid);
	virtual ~Item();

	virtual QSize sizeHint( const QStyleOptionViewItem &option) const;

	inline int getHeight() const { return m_height;}

	virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

	inline const QString& getName()              const { return m_name;}   ///< Get item name.

	virtual inline const QVariant getToolTip()     const { return QVariant(); }
	virtual inline const QString getSelectString() const { return m_name; }///< Get item select string.

	inline void lock()                const { setLock( true);  }
	inline void unLock()              const { setLock( false); }
	inline bool isLocked()            const { return m_locked;   }
	inline void setLock( bool value ) const { m_locked = value;  }

	inline int getId() const { return m_id; }
	inline void resetId() { m_id = 0; }

	const QColor & clrTextMain(      const QStyleOptionViewItem &option)           const;
	const QColor & clrTextInfo(      const QStyleOptionViewItem &option)           const;
	const QColor & clrTextDone(      const QStyleOptionViewItem &option)           const;
	const QColor & clrTextMuted(     const QStyleOptionViewItem &option)           const;
	const QColor & clrTextState(     const QStyleOptionViewItem &option, bool on ) const;

	static const QColor & clrTextInfo( bool running, bool selected, bool locked);

	inline void      setRunning() const { m_running = true;  } ///< Set item m_running ( to change text colors only).
	inline void   setNotRunning() const { m_running = false; } ///< Set item not m_running ( to change text colors only).
	inline const bool isRunning() const { return m_running;  } ///< Return whether node is "m_running".

	static const QString generateErrorsSolvingInfo( int i_block, int i_task, int i_retries);

	/// Draw a percent bar.
	static void drawPercent
	(
		QPainter * painter,
		int posx, int posy, int width, int height,
		int whole,
		int part_a, int part_b = 0, int part_c = 0,
		bool drawBorder = true
	);

	static void calcutaleStarPoints();

	inline static bool  isItemP(const QVariant & i_v) { return i_v.canConvert<Item*>(); }
	inline static Item* toItemP(const QVariant & i_v) { return i_v.value<Item*>(); }

	virtual void v_filesReceived( const af::MCTaskUp & i_taskup);

	virtual bool calcHeight();
	
protected:
	void drawBack( QPainter *painter, const QStyleOptionViewItem &option, const QColor * i_clrItem = NULL, const QColor * i_clrBorder = NULL) const;

	/// Print AFJOB::STATE informaton
	void printfState( const uint32_t state, int posx, int posy, QPainter * painter, const QStyleOptionViewItem &option) const;

	/// Draw a star at \c x,y coordinates with \c size size.
	static void drawStar( int size, int posx, int posy, QPainter * painter);

protected:
	static const int Height;
	static const int Width;

protected:
	QString m_name;           ///< Item name.

	int m_height;

	mutable bool m_locked;

private:
	/// Node "m_running" property.
	mutable bool m_running;

	/// Points contains star coordinates.
	static QPolygonF ms_star_pointsInit;

	/// Points used to offset and scale default star coordinates.
	static QPolygonF ms_star_pointsDraw;

	int m_id;
};

Q_DECLARE_METATYPE( Item*)
