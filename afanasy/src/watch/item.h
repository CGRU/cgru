#pragma once

#include "../libafanasy/afnode.h"

#include <QtGui/QItemDelegate>
#include <QtGui/QStyleOption>

class MainWidget;

class Item
{
public:
    Item( const QString &itemname, int itemid);
    virtual ~Item();

    virtual QSize sizeHint( const QStyleOptionViewItem &option) const;

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

    inline const QString& getName()              const { return name;}   ///< Get item name.

    virtual inline const QVariant getToolTip()     const { return QVariant(); }
    virtual inline const QString getSelectString() const { return name; }///< Get item select string.

    inline void lock()                const { setLock( true);  }
    inline void unLock()              const { setLock( false); }
    inline bool isLocked()            const { return locked;   }
    inline void setLock( bool value ) const { locked = value;  }

    inline int getId() const { return id; }
    inline void resetId() { id = 0; }

    const QColor & clrTextMain(      const QStyleOptionViewItem &option)           const;
    const QColor & clrTextInfo(      const QStyleOptionViewItem &option)           const;
    const QColor & clrTextDone(      const QStyleOptionViewItem &option)           const;
    const QColor & clrTextMuted(     const QStyleOptionViewItem &option)           const;
    const QColor & clrTextState(     const QStyleOptionViewItem &option, bool on ) const;

    static const QColor & clrTextInfo( bool running, bool selected, bool locked);

    inline void      setRunning() const { running = true;  } ///< Set item running ( to change text colors only).
    inline void   setNotRunning() const { running = false; } ///< Set item not running ( to change text colors only).
    inline const bool isRunning() const { return running;  } ///< Return whether node is "running".

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

    virtual bool calcHeight();

protected:
    virtual void drawBack( QPainter *painter, const QStyleOptionViewItem &option) const;
    virtual void drawPost( QPainter *painter, const QStyleOptionViewItem &option, float alpha = 1.0) const;

    /// Print AFJOB::STATE informaton
    void printfState( const uint32_t state, int posx, int posy, QPainter * painter, const QStyleOptionViewItem &option) const;

    /// Draw a star at \c x,y coordinates with \c size size.
    static void drawStar( int size, int posx, int posy, QPainter * painter);

protected:
    QString name;           ///< Item name.

    static const int Height;
    static const int Width;

    mutable bool locked;

private:
    /// Node "running" property.
    mutable bool running;

    /// Points contains star coordinates.
    static QPolygonF ms_star_pointsInit;

    /// Points used to offset and scale default star coordinates.
    static QPolygonF ms_star_pointsDraw;

    int id;
};

Q_DECLARE_METATYPE( Item*)
