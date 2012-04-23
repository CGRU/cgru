#pragma once

#include "../libafqt/qenvironment.h"

#include <QtGui/QColor>
#include <QtGui/QPainter>

class Plotter
{
public:
   Plotter( int Numgrpaphs, int Scale = 100, int NumLinesWidth = 128);
   ~Plotter();

   inline bool isValid() const { return valid; }

   void setColor(    int r, int g, int b, int grp = 0);
   void setColorHot( int r, int g, int b, int grp = 0);
   inline void setScale( int Scale) { scale = Scale; if( scale < 1) autoscale = true; else autoscale = false;}
   inline void setAutoScaleMax( int MaxScale) { autoscale_maxscale = MaxScale; autoscale_maxbgc = MaxScale;}
   inline void setAutoScaleMaxBGC( int MaxScale) { autoscale_maxbgc = MaxScale;}
   inline void setAutoScaleLines( int Lines)  { autoscale_lines = Lines; }
   inline void setLabel( const QString & Label) { label = Label; }
   inline void setBGColor(    int r, int g, int b) { bgc_r=r; bgc_g=g; bgc_b=b; }
   inline void setLabelColor( int r, int g, int b) { label_color.setRgb( r, g, b); }
   inline void setLabelFontSize( int size) { label_font.setPointSize( size); }
   inline void setHotMin( int HotMin, int grp = 0) { hot_min[grp] = HotMin; }
   inline void setHotMax( int HotMax, int grp = 0) { hot_max[grp] = HotMax; }

   void addValue( int grp, int val, bool store = true);
   inline void setLabelValue( int Value) { label_value = Value; }
   inline void addLabelText( const QString & text) { label_text += text; }

   void paint( QPainter * painter, int x, int y, int w, int h) const;

   int height;

private:
   QString label;
   int label_value;
   mutable QString label_text;

   bool  autoscale;
   int   autoscale_lines;
   int   autoscale_maxscale;
   int   autoscale_maxbgc;
   int * hot_min;
   int * hot_max;

   int graphs;
   int lines;
   int scale;

   int * clrR_cur;
   int * clrR_cur_hot;
   int * clrG_cur;
   int * clrG_cur_hot;
   int * clrB_cur;
   int * clrB_cur_hot;

   QColor label_color;
   QFont  label_font;

   int ** values;
   int ** clr_r;
   int ** clr_g;
   int ** clr_b;

   int bgc_r;
   int bgc_g;
   int bgc_b;

   bool valid;

private:
   void calcHot( int grp, int value, int &r, int &g, int &b) const;
};
