#include "plotter.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Plotter::Plotter( int NumGraphs, int Scale, int NumLinesWidth):
   label("%1"),
   label_value( 1),
   autoscale( false),
   autoscale_lines( 32),
   autoscale_maxscale( 1<<30),
   graphs( NumGraphs),
   lines( NumLinesWidth),
   scale( Scale),
   data( NULL),
   valid( false)
{
   if( graphs < 1)
   {
      AFERRAR("Plotter::Plotter: NumGraphs < 1 ( %d < 1 )\n", graphs);
      return;
   }
   if( lines < 1)
   {
      AFERRAR("Plotter::Plotter: NumLinesWidth < 1 ( %d < 1 )\n", lines);
      return;
   }
   if( scale < 1) autoscale = true;

   hot_min      = new int [ graphs];
   hot_max      = new int [ graphs];
   clrR_cur     = new int [ graphs];
   clrR_trl     = new int [ graphs];
   clrR_cur_hot = new int [ graphs];
   clrR_trl_hot = new int [ graphs];
   clrG_cur     = new int [ graphs];
   clrG_trl     = new int [ graphs];
   clrG_cur_hot = new int [ graphs];
   clrG_trl_hot = new int [ graphs];
   clrB_cur     = new int [ graphs];
   clrB_trl     = new int [ graphs];
   clrB_cur_hot = new int [ graphs];
   clrB_trl_hot = new int [ graphs];

   data = new int*[ graphs];
   int cdelta = 234 / (graphs+1);
   for( int grp = 0; grp < graphs; grp++)
   {
      data[grp] = new int[lines];
      for( int l = 0; l < lines; l++) data[grp][l] = 0;
      int color = (grp+1) * cdelta;
      setColor( color>>1, color, color>>2, grp);
      hot_min[grp] = 0;
      hot_max[grp] = 0;
   }
   bgcolor.setRgb( 10, 20, 5);
   label_color.setRgb( 150, 250, 50);
   valid = true;
}

Plotter::~Plotter()
{
   for( int grp = 0; grp < graphs; grp++) if( data[grp] ) delete [] data[grp];
   delete [] data;
   delete [] hot_min;
   delete [] hot_max;
   delete [] clrR_cur;
   delete [] clrR_trl;
   delete [] clrR_cur_hot;
   delete [] clrR_trl_hot;
   delete [] clrG_cur;
   delete [] clrG_trl;
   delete [] clrG_cur_hot;
   delete [] clrG_trl_hot;
   delete [] clrB_cur;
   delete [] clrB_trl;
   delete [] clrB_cur_hot;
   delete [] clrB_trl_hot;
}

void Plotter::setColor( int r, int g, int b, int grp)
{
   if( grp >= graphs)
   {
      AFERRAR("Plotter::setColor: num >= graphs ( %d >= %d)\n", grp, graphs);
      return;
   }
   clrR_cur[grp] = r;
   clrG_cur[grp] = g;
   clrB_cur[grp] = b;
   clrR_trl[grp] = (2*r)/3;
   clrG_trl[grp] = (2*g)/3;
   clrB_trl[grp] = (2*b)/3;
}

void Plotter::setColorHot( int r, int g, int b, int grp)
{
   if( grp >= graphs)
   {
      AFERRAR("Plotter::setColor: grp >= graphs ( %d >= %d)\n", grp, graphs);
      return;
   }
   clrR_cur_hot[grp] = r;
   clrG_cur_hot[grp] = g;
   clrB_cur_hot[grp] = b;
   clrR_trl_hot[grp] = (2*r)/3;
   clrG_trl_hot[grp] = (2*g)/3;
   clrB_trl_hot[grp] = (2*b)/3;
}

void Plotter::addValue( int grp, int val )
{
   if( grp >= graphs)
   {
      AFERRAR("Plotter::addValue: grp >= graphs ( %d >= %d)\n", grp, graphs);
      return;
   }
   for( int l = 1; l < lines; l++) data[grp][l-1] = data[grp][l];
   data[grp][lines-1] = val;

   // Calculate scale
   if( autoscale)
   {
      int maxvalue = 0;
      for( int l = lines - autoscale_lines; l < lines; l++)
      {
         int sum = 0;
         for( int grp = 0; grp < graphs; grp++) sum += data[grp][l];
         if( maxvalue < sum) maxvalue = sum;
      }
      scale = 1;
      while((scale < maxvalue) && (scale < autoscale_maxscale)) scale *= 10;
      float scale_val = float(scale) / (label_value != 0 ? label_value : 1);
      QString scale_str = QString("%1").arg( scale_val);
      if( scale_str.indexOf('0') == 0) scale_str = scale_str.remove( 0, 1);
      label_text = label.arg( scale_str);
   }
}

void Plotter::paint( QPainter * painter, int x, int y, int w, int h) const
{
   int width = 8;
   if( w > lines+width) w = lines+width;
   painter->setPen( Qt::NoPen );
   painter->setBrush( QBrush( bgcolor, Qt::SolidPattern ));
   painter->drawRect( x, y, w, h);

   int line_y = y+h;
   for( int grp = 0; grp < graphs; grp++)
   {
      painter->setBrush( QBrush( getColor(grp, data[grp][lines-1]), Qt::SolidPattern ));
      int line_h = (h*data[grp][lines-1]) / (scale != 0 ? scale : 1);
      painter->drawRect( x+w-width, line_y - line_h, width, line_h);
      line_y -= line_h;
   }

   int line_x = x;
   int start = lines - w + width;
   if( start < 0) start = 0;
   for( int l = start; l < lines; l++)
   {
      line_y = y+h;
      for( int grp = 0; grp < graphs; grp++)
      {
         painter->setPen( getColor( grp,  data[grp][l], true));
         int line_h = (h*data[grp][l]) / (scale != 0 ? scale : 1);
         if( line_y <= y ) break;
         int line_yh = line_y - line_h;
         painter->drawLine( line_x, line_y, line_x, line_yh > y ? line_yh : y);
         line_y -= line_h;
      }
      line_x ++;
   }

   painter->setPen( label_color);
   if( autoscale == false ) label_text = label.contains('%') ? label.arg( label_value) : label;
   painter->drawText( x+1, y+10, label_text);
}

const QColor Plotter::getColor(  int grp, int value, bool trail) const
{
   int r = clrR_cur[grp];
   int g = clrG_cur[grp];
   int b = clrB_cur[grp];
   int hr = clrR_cur_hot[grp];
   int hg = clrG_cur_hot[grp];
   int hb = clrB_cur_hot[grp];
   if( trail)
   {
      r  = clrR_trl[grp];
      g  = clrG_trl[grp];
      b  = clrB_trl[grp];
      hr = clrR_trl_hot[grp];
      hg = clrG_trl_hot[grp];
      hb = clrB_trl_hot[grp];
   }

   if( hot_max[grp] == 0 )    return QColor(  r,  g,  b);
   if( value <= hot_min[grp]) return QColor(  r,  g,  b);
   if( value >= hot_max[grp]) return QColor( hr, hg, hb);

   int hot_max_min = hot_max[grp] -hot_min[grp];
   if( hot_max_min == 0) hot_max_min = 1;
   float factor = (1.0 * (value - hot_min[grp])) / hot_max_min;
   r = (float)r*(1.0 - factor)/1 + (float)hr * factor / 1;
   g = (float)g*(1.0 - factor)/1 + (float)hg * factor / 1;
   b = (float)b*(1.0 - factor)/1 + (float)hb * factor / 1;

   return QColor( r, g, b);
}
