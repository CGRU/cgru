#pragma once

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPalette>

#include "name_afqt.h"
#include "attr.h"
#include "attrnumber.h"
#include "attrcolor.h"

class QByteArray;
class QDomElement;

class afqt::QEnvironment
{
public:
   QEnvironment( const QString & name);
   ~QEnvironment();

   static inline bool isValid() { return valid;}

   static void initFonts();

   static void setPalette( QPalette & palette);

   static const QString getFileName() { return filename; }

   static bool save();
   static void saveWndRects( QByteArray & data);
   static void saveGUI( QByteArray & data);

   static bool getRect( const QString & name, QRect & rect);
   static void setRect( const QString & name, const QRect & rect);

   static AttrNumber savePrefsOnExit;
   static AttrNumber saveWndRectsOnExit;
   static AttrNumber saveGUIOnExit;
   static AttrNumber showOfflineNoise;

   static AttrColor clr_Window;
   static AttrColor clr_WindowText;
   static AttrColor clr_Base;
// static   AttrColor clr_AlternateBase;
   static AttrColor clr_Text;
   static AttrColor clr_Button;

   static AttrColor clr_Light;
   static AttrColor clr_Midlight;
   static AttrColor clr_Dark;
   static AttrColor clr_Mid;
   static AttrColor clr_Shadow;

   static AttrColor clr_Highlight;
   static AttrColor clr_HighlightedText;
   static AttrColor clr_Link;              // Visor mode
   static AttrColor clr_LinkVisited;       // GOD mode

   static AttrColor clr_item;
   static AttrColor clr_selected;
   static AttrColor clr_itemjob;
   static AttrColor clr_itemjoboff;
   static AttrColor clr_itemjobwtime;
   static AttrColor clr_itemjobwdep;
   static AttrColor clr_itemjobdone;
   static AttrColor clr_itemjoberror;
   static AttrColor clr_itemrender;
   static AttrColor clr_itemrenderoff;
   static AttrColor clr_itemrenderbusy;
   static AttrColor clr_itemrendernimby;
   static AttrColor clr_error;
   static AttrColor clr_running;
   static AttrColor clr_done;
   static AttrColor clr_outline;
   static AttrColor clr_star;
   static AttrColor clr_starline;

   static AttrColor clr_textmuted;
   static AttrColor clr_textbright;
   static AttrColor clr_textdone;
   static AttrColor clr_textstars;

   static Attr font_family;

   static AttrNumber font_sizename;
   static AttrNumber font_sizeinfo;
   static AttrNumber font_sizemin;

   static QColor qclr_black;
   static QColor qclr_white;

   static QFont f_name;
   static QFont f_info;
   static QFont f_min;


private:
   static bool valid;
   static QString filename;
   static QList<Attr*> attrs_prefs;
   static QList<AttrRect*> attrs_wndrects;
   static QList<Attr*> attrs_gui;
};
