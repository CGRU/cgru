#pragma once

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
   inline void setAutoScaleMax( int MaxScale) { autoscale_maxscale = MaxScale; }
   inline void setLabel( const QString & Label) { label = Label; }
   inline void setLabelColor( int r, int g, int b) { label_color.setRgb( r, g, b); }
   inline void setBGColor( int r, int g, int b) { bgcolor.setRgb( r, g, b); }
   inline void setHotMin( int HotMin, int grp = 0) { hot_min[grp] = HotMin; }
   inline void setHotMax( int HotMax, int grp = 0) { hot_max[grp] = HotMax; }

   void addValue( int grp, int val );
   inline void setLabelValue( int Value) { label_value = Value; }

   void paint( QPainter * painter, int x, int y, int w, int h) const;

private:
   QString label;
   int label_value;
   mutable QString label_text;

   bool  autoscale;
   int   autoscale_lines;
   int   autoscale_maxscale;
   int * hot_min;
   int * hot_max;

   int graphs;
   int lines;
   int scale;

   int * clrR_cur;
   int * clrR_trl;
   int * clrR_cur_hot;
   int * clrR_trl_hot;
   int * clrG_cur;
   int * clrG_trl;
   int * clrG_cur_hot;
   int * clrG_trl_hot;
   int * clrB_cur;
   int * clrB_trl;
   int * clrB_cur_hot;
   int * clrB_trl_hot;

   QColor bgcolor;
   QColor label_color;

   int ** data;

   bool valid;

private:
   const QColor getColor(  int grp, int value, bool trail = false) const;
};
