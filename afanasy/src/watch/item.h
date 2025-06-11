#pragma once

#include "../libafanasy/afnode.h"

#include <QItemDelegate>
#include <QMenu>
#include <QStyleOption>

class ItemButton;
class MainWidget;

class Item
{
  public:
	enum EType
	{
		TNone = 0,

		TBlock = 1,
		TBranch = 2,
		TJob = 3,
		TMonitor = 4,
		TPool = 5,
		TRender = 6,
		TTask = 7,
		TUser = 8,

		TAny = 99
	};

	Item(const QString &i_name, int i_id, EType i_type);
	virtual ~Item();

	static const int HeightAnnotation;
	static const int HeightTickets;

	virtual QSize sizeHint(const QStyleOptionViewItem &option) const;

	inline int getHeight() const { return m_height; }

	virtual bool v_isSelectable() const;

	void paint(QPainter *i_painter, const QStyleOptionViewItem &i_option) const;
	virtual void v_paint(QPainter *i_painter, const QRect &i_rect,
						 const QStyleOptionViewItem &i_option) const;

	bool mousePressed(const QPoint &i_point, const QRect &i_rect, const Qt::MouseButtons &i_buttons);
	virtual bool v_mousePressed(int i_x, int i_y, int i_w, int i_h, const Qt::MouseButtons &i_buttons);

	virtual void v_buttonClicked(ItemButton *i_b);

	inline const QString &getName() const { return m_name; } ///< Get item name.

	virtual const QVariant v_getToolTip() const { return QVariant(); }
	virtual const QString v_getSelectString() const { return m_name; }
	virtual const QString v_getInfoText() const { return m_info_text; }

	virtual const QString v_getMultiSelecedText(const QList<Item *> &i_selected) const { return QString(); }

	inline void lock() const { setLock(true); }
	inline void unLock() const { setLock(false); }
	inline bool isLocked() const { return m_locked; }
	inline void setLock(bool value) const { m_locked = value; }

	inline int getId() const { return m_id; }

	// Item can decide to delete itself.
	// It can set its ID to zero.
	// ListNodes deletes such items on update.
	inline void resetId() { m_id = 0; }

	inline EType getType() const { return m_type; }

	const QColor &clrTextMain(const QStyleOptionViewItem &option) const;
	const QColor &clrTextInfo(const QStyleOptionViewItem &option) const;
	const QColor &clrTextDone(const QStyleOptionViewItem &option) const;
	const QColor &clrTextMuted(const QStyleOptionViewItem &option) const;
	const QColor &clrTextState(const QStyleOptionViewItem &option, bool on) const;

	static const QColor &clrTextInfo(bool running, bool selected, bool locked = false);

	inline void setRunning() const
	{
		m_running = true;
	} ///< Set item m_running ( to change text colors only).
	inline void setNotRunning() const
	{
		m_running = false;
	} ///< Set item not m_running ( to change text colors only).
	inline bool isRunning() const { return m_running; } ///< Return whether node is "m_running".

	static const QString generateErrorsSolvingInfo(int i_block, int i_task, int i_retries);

	/// Draw a percent bar.
	static void drawPercent(QPainter *painter, int posx, int posy, int width, int height, int whole,
							int part_a, int part_b = 0, int part_c = 0, bool drawBorder = true,
							QColor *i_done_color = NULL);

	static void calcutaleStarPoints();

	inline static bool isItemP(const QVariant &i_v) { return i_v.canConvert<Item *>(); }
	inline static Item *toItemP(const QVariant &i_v) { return i_v.value<Item *>(); }

	virtual void v_filesReceived(const af::MCTaskUp &i_taskup);

	virtual bool calcHeight();

	inline void setHidden(bool i_hidden)
	{
		v_processHidden(i_hidden);
		m_hidden = i_hidden;
	}
	inline bool isHidden() const { return m_hidden; }

	// Job item gets thumbnail here:
	virtual void v_processHidden(bool i_hidden) {};

	inline bool hasParam(const QString &i_name) const { return m_params.contains(i_name); }
	const QVariant &getParamVar(const QString &i_name) const;

	enum ETK_DRAW_OPTS
	{
		TKD_LEFT = 1 << 0,
		TKD_RIGHT = 1 << 1,
		TKD_BORDER = 1 << 2,
		TKD_DUMMY = 1 << 3
	};

	/// Draw ticket and return its width
	static int drawTicket(QPainter *i_painter, const QPen &i_text_pen, int i_x, int i_y, int i_w, int i_h,
						  int i_opts, const QString &i_name, int i_count, int i_usage = -1, int i_hosts = -1,
						  int i_max_hosts = -1);

	inline void addButton(ItemButton *i_ib) { m_buttons.append(i_ib); }

	// Called on delete ids event.
	// Not called on list dtor.
	virtual void v_toBeDeleted();

  protected:
	inline int getDepth() const { return m_depth; }
	void setDepth(int i_depth);

	void drawBack(QPainter *i_painter, const QRect &i_rect, const QStyleOptionViewItem &i_option,
				  const QColor *i_clrItem = NULL, const QColor *i_clrBorder = NULL) const;

	/// Print AFJOB::STATE informaton
	void printfState(const uint32_t state, int posx, int posy, QPainter *painter,
					 const QStyleOptionViewItem &option) const;

	/// Draw a star at \c x,y coordinates with \c size size.
	static void drawStar(int size, int posx, int posy, QPainter *painter);

  protected:
	static const int Height;
	static const int Width;

  protected:
	QString m_name; ///< Item name.

	QString m_info_text;

	QMap<QString, QVariant> m_params;

	int m_height;

	mutable bool m_locked;

  private:
	int m_depth;
	int m_margin_left;
	static const int DepthOffset = 24;

	/// Node "m_running" property.
	mutable bool m_running;

	QVector<ItemButton *> m_buttons;

	/// Points contains star coordinates.
	static QPolygonF ms_star_pointsInit;

	/// Points used to offset and scale default star coordinates.
	static QPolygonF ms_star_pointsDraw;

	bool m_hidden;

	int m_id;

	EType m_type;
};

Q_DECLARE_METATYPE(Item *)
