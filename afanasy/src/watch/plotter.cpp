#include "plotter.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Plotter::Plotter( int NumGraphs, int Scale, int NumLinesWidth):
   height(0),
   label("%1"),
   label_value( 1),
   autoscale( false),
   autoscale_lines( 32),
   autoscale_maxscale( 1<<30),
   graphs( NumGraphs),
   lines( NumLinesWidth),
   scale( Scale),
   values( NULL),
   bgc_r(0), bgc_g(0), bgc_b(0),
   valid( false)
{
   if( graphs < 1)
   {
      AFERRAR("Plotter::Plotter: NumGraphs < 1 ( %d < 1 )", graphs)
      return;
   }
   if( lines < 1)
   {
      AFERRAR("Plotter::Plotter: NumLinesWidth < 1 ( %d < 1 )", lines)
      return;
   }
   if( scale < 1) autoscale = true;

   hot_min      = new int [ graphs];
   hot_max      = new int [ graphs];
   clrR_cur     = new int [ graphs];
   clrR_cur_hot = new int [ graphs];
   clrG_cur     = new int [ graphs];
   clrG_cur_hot = new int [ graphs];
   clrB_cur     = new int [ graphs];
   clrB_cur_hot = new int [ graphs];
   autoscale_maxbgc =  autoscale_maxscale;

   values = new int*[ graphs];
   clr_r  = new int*[ graphs];
   clr_g  = new int*[ graphs];
   clr_b  = new int*[ graphs];
   int cdelta = 234 / (graphs+1);
   for( int grp = 0; grp < graphs; grp++)
   {
      values[grp] = new int[lines];
      clr_r[grp]  = new int[lines];
      clr_g[grp]  = new int[lines];
      clr_b[grp]  = new int[lines];
      for( int l = 0; l < lines; l++)
      {
         values[grp][l] = 0;
         clr_r[grp][l]  = 0;
         clr_g[grp][l]  = 0;
         clr_b[grp][l]  = 0;
      }
      int color = (grp+1) * cdelta;
      setColor( color>>1, color, color>>2, grp);
      hot_min[grp] = 0;
      hot_max[grp] = 0;
      setColorHot( 255, 0, 0, grp);
   }

   label_font = afqt::QEnvironment::f_plotter;

   valid = true;
}

Plotter::~Plotter()
{
   for( int grp = 0; grp < graphs; grp++)
   {
      delete [] values[grp];
      delete []  clr_r[grp];
      delete []  clr_g[grp];
      delete []  clr_b[grp];
   }
   delete [] values;
   delete [] clr_r;
   delete [] clr_g;
   delete [] clr_b;
   delete [] hot_min;
   delete [] hot_max;
   delete [] clrR_cur;
   delete [] clrR_cur_hot;
   delete [] clrG_cur;
   delete [] clrG_cur_hot;
   delete [] clrB_cur;
   delete [] clrB_cur_hot;
}

void Plotter::setColor( int r, int g, int b, int grp)
{
   if( grp >= graphs)
   {
      AFERRAR("Plotter::setColor: num >= graphs ( %d >= %d)", grp, graphs)
      return;
   }
   clrR_cur[grp] = r;
   clrG_cur[grp] = g;
   clrB_cur[grp] = b;
}

void Plotter::setColorHot( int r, int g, int b, int grp)
{
   if( grp >= graphs)
   {
      AFERRAR("Plotter::setColor: grp >= graphs ( %d >= %d)", grp, graphs)
      return;
   }
   clrR_cur_hot[grp] = r;
   clrG_cur_hot[grp] = g;
   clrB_cur_hot[grp] = b;
}

void Plotter::addValue( int grp, int val, bool store)
{
   if( grp >= graphs)
   {
      AFERRAR("Plotter::addValue: grp >= graphs ( %d >= %d)", grp, graphs)
      return;
   }

   // Shift previous data to store (if needed):
   if( store)
      for( int l = 1; l < lines; l++)
      {
         values[grp][l-1] = values[grp][l];
         clr_r[ grp][l-1] =  clr_r[grp][l];
         clr_g[ grp][l-1] =  clr_g[grp][l];
         clr_b[ grp][l-1] =  clr_b[grp][l];
      }

   // Store current value:
   values[grp][lines-1] = val;

   // Store current color:
   int r = clrR_cur[grp];
   int g = clrG_cur[grp];
   int b = clrB_cur[grp];
   if( hot_max[grp] != 0 ) calcHot( grp, val, r,g,b);
   clr_r[ grp][lines-1] = r;
   clr_g[ grp][lines-1] = g;
   clr_b[ grp][lines-1] = b;

   // Calculate auto scale:
   if( autoscale)
   {
      // Search for max value in 'autoscale_lines':
      int maxvalue = 0;
      for( int l = lines - autoscale_lines; l < lines; l++)
      {
         int sum = 0;
         // Sum all grapphs values:
         for( int grp = 0; grp < graphs; grp++) sum += values[grp][l];
         if( maxvalue < sum) maxvalue = sum;
      }

      // Calculate scale:
      scale = 1;
      int scale_add = 1;
      int loop = 1;
      while( scale < autoscale_maxscale )
      {
         if( scale >= maxvalue) break;
         if( loop >= 10 )
         {
            scale_add *= 10;
            loop = 1;
         }
         scale += scale_add;
         loop ++;
      }

      // Append label text by autoscale value:
      float scale_val = float(scale) / (label_value != 0 ? label_value : 1);
      QString scale_str = QString("%1").arg( scale_val);
      if( scale_str.indexOf('0') == 0) scale_str = scale_str.remove( 0, 1);
      label_text = label.arg( scale_str);

      // Calculate blue backgroud color:
      int value = scale;
      bgc_b = 128;
      while( value < autoscale_maxbgc ) { value *= 10; bgc_b = bgc_b >> 1;}
      if( bgc_b <   0 ) bgc_b = 0;
      if( bgc_b > 255 ) bgc_b = 255;
   }
}

void Plotter::paint( QPainter * painter, int x, int y, int w, int h) const
{
   int width = 8;
   if( w > lines+width) w = lines+width;
   painter->setPen( Qt::NoPen );
   painter->setBrush( QBrush( QColor( bgc_r, bgc_g, bgc_b), Qt::SolidPattern ));
   painter->drawRect( x, y, w, h);

   int line_y = y+h;
   for( int grp = 0; grp < graphs; grp++)
   {
      painter->setBrush( QBrush( QColor( clr_r[grp][lines-1],clr_g[grp][lines-1],clr_b[grp][lines-1]), Qt::SolidPattern ));
      int line_h = (h*values[grp][lines-1]) / (scale != 0 ? scale : 1);
      if( line_y - line_h < y ) line_h = line_y - y;
      if( line_h < 1 ) continue;
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
         int r = ( 2 * clr_r[grp][l] ) / 3;
         int g = ( 2 * clr_g[grp][l] ) / 3;
         int b = ( 2 * clr_b[grp][l] ) / 3;
         painter->setPen( QColor( r,g,b));
         int line_h = (h*values[grp][l]) / (scale != 0 ? scale : 1);
         if( line_y <= y ) break;
         int line_yh = line_y - line_h;
         painter->drawLine( line_x, line_y, line_x, line_yh > y ? line_yh : y);
         line_y -= line_h;
      }
      line_x ++;
   }

   if( label_color.isValid())
      painter->setPen(  label_color);
   else
      painter->setPen(  afqt::QEnvironment::clr_itemrenderpltclr.c);
   painter->setFont( label_font);

//   if( autoscale == false ) label_text = label;//.contains("%1") ? label.arg( label_value) : label;
   painter->drawText( x+1, y+1, w, h, Qt::AlignLeft | Qt::AlignTop, label_text.isEmpty() ? label : label_text);

   x -= 1; y -= 1; w += 2; h +=2;

   painter->setOpacity( 0.3);
   painter->setPen( afqt::QEnvironment::qclr_white );
   painter->drawLine( x, y+h, x+w-1, y+h);

   painter->setOpacity( 0.5);
   painter->setPen( afqt::QEnvironment::qclr_black );
   painter->drawLine( x, y+1, x, y+h-1);
   painter->drawLine( x+w-1, y+1, x+w-1, y+h-1);
   painter->drawLine( x, y, x+w-1, y);

   painter->setOpacity( 1.0);
}

void Plotter::calcHot( int grp, int value, int &r, int &g, int &b) const
{
	if(( hot_min[grp] == 0 ) || ( hot_max[grp] == 0 )) return;

   if( value <= hot_min[grp]) return;

   int hr = clrR_cur_hot[grp];
   int hg = clrG_cur_hot[grp];
   int hb = clrB_cur_hot[grp];

   if( value >= hot_max[grp])
   {
      r = hr; g = hg; b = hb;
      return;
   }

   int hot_max_min = hot_max[grp] -hot_min[grp];
   if( hot_max_min < 1) hot_max_min = 1;
   float factor = (1.0 * (value - hot_min[grp])) / (float)hot_max_min;
   r = int( (float)r*(1.0 - factor) + (float)hr * factor );
   g = int( (float)g*(1.0 - factor) + (float)hg * factor );
   b = int( (float)b*(1.0 - factor) + (float)hb * factor );
}
