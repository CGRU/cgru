#include "offlinescreen.h"

#include "watch.h"

#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

typedef union _rgba
{
   uint8_t c[4];
   uint32_t all;
} RGBA;
int R = 2;
int G = 1;
int B = 0;
int A = 3;

OfflineScreen::OfflineScreen( QWidget * widget):
   QWidget((QWidget*)(Watch::getDialog())),
   offlineimage( afqt::stoq( af::Environment::getAfRoot()) + "/icons/watch/offlineimage.png")
{
   // Allocate memory for image
   image_w = widget->width();
   image_h = widget->height();
   image_s = image_w * image_h;
   image = new uint32_t[image_s];
   buffer_a = new uint32_t[image_s];
   buffer_b = new uint32_t[image_s];

   // Grab widget:
   QImage qimage( image_w, image_h, QImage::Format_RGB32);
#if QT_VERSION >= 0x040300
   widget->render( &qimage);
#else
   qimage.fill( 0xff808080);
#endif

   // Draw border:
   QPainter painter( &qimage);
   static const int border = 30;
   for( int i = 0; i < border; i++)
   {
      painter.setPen( QColor(0,0,0,250 - i*250/border));
      painter.drawRect( i, i, image_w - (i<<1) - 1, image_h - (i<<1) - 1);
   }

   // Save image data:
   for( int y = 0; y < image_h; y++)
   for( int x = 0; x < image_w; x++)
   {
      QRgb qrgb = qimage.pixel( x, y);
      RGBA rgba;
      rgba.c[A] = 0xff;
      rgba.c[R] = qRed( qrgb);
      rgba.c[G] = qGreen( qrgb);
      rgba.c[B] = qBlue( qrgb);
      buffer_a[ y*image_w + x ] = rgba.all;
   }

   // Echo screen:
   int echo = int(-10.0f - 10.0f * rand() / RAND_MAX);
   for( int h = 0; h < image_h; h++)
   for( int w = 0; w < image_w; w++)
   {
      int src = h*image_w + w;
      int f_w = w + echo;
      if( f_w < 0 ) f_w += image_w;
      if( f_w >= image_w ) f_w -= image_w;
      int dest = h*image_w + f_w;
      RGBA rgba_src, rgba_echo, rgba_dest;
      rgba_src.all = buffer_a[src];
      rgba_echo.all = buffer_a[dest];
      int offset = 70;
      for( int c = 0; c < 4; c ++)
      {
         if( c == A ) continue;
         rgba_dest.c[c] = rgba_src.c[c] * 7 / 10 + rgba_echo.c[c] * 3 / 10;
         if( rgba_dest.c[c] > 40 ) offset = 0;
      }
      if( offset)
         for( int c = 0; c < 4; c ++)
         {
            if( c == A ) continue;
            rgba_dest.c[c] += offset;
         }
      image[src] = rgba_dest.all;
   }

   // Fill pseudo randoms:
   rand_all = new int[image_s];
   for( int i = 0; i < image_s; i++) rand_all[i] = rand();
   rand_all_size = new int[image_s];
   for( int i = 0; i < image_s; i++) rand_all_size[i] = int(float(rand()) * (image_s - 1.0f) / float(RAND_MAX));
   // All constant sum (grain):
   rand_all_grain = new int[image_s];
   for( int i = 0; i < image_s; i++)
   {
      RGBA rgba;
      int c;
      c = int(  40.0f * rand() / RAND_MAX + 80.0f); rgba.c[R] = c;
      c = int( 100.0f * rand() / RAND_MAX + 50.0f); rgba.c[G] = c;
      c = int( 200.0f * rand() / RAND_MAX + 20.0f); rgba.c[B] = c;
      rand_all_grain[i] = rgba.all;
   }
   // Black & White points:
   bwsize = 4;
   rand_bw_len = new int[bwsize];
   rand_bw_max = new int[bwsize];
   rand_bw_pos = new int*[bwsize];
   rand_bw_len[0] = image_s >> 3;
   rand_bw_max[0] = image_s >> 1;
   rand_bw_len[1] = image_s >> 3;
   rand_bw_max[1] = image_s >> 2;
   rand_bw_len[2] = image_s >> 4;
   rand_bw_max[2] = image_s >> 4;
   rand_bw_len[3] = image_s >> 5;
   rand_bw_max[3] = image_s >> 6;
   for( int bw = 0; bw < bwsize; bw++)
   {
      rand_bw_pos[bw] = new int[rand_bw_len[bw]];
      for( int i = 0; i < rand_bw_len[bw]; i++)
         rand_bw_pos[bw][i] = int( float(rand()) * rand_bw_max[bw] / float(RAND_MAX));
   }
   rand_bw_val = new int[image_s];
   for( int i = 0; i < image_s; i++) rand_bw_val[i] = int( 200.0f * rand() / RAND_MAX);

   // Create timer to refresh screen:
   timer = new QTimer( this);
   connect( timer, SIGNAL( timeout()), this, SLOT( repaint()));
   timer->start( 1000 / 20 );
}

OfflineScreen::~OfflineScreen()
{
   delete timer;
   delete [] image;
   delete [] buffer_a;
   delete [] buffer_b;
   delete [] rand_all;
   delete [] rand_all_size;
   delete [] rand_all_grain;
   for( int i = 0; i < bwsize; i++) delete [] rand_bw_pos[i];
   delete [] rand_bw_pos;
   delete [] rand_bw_len;
   delete [] rand_bw_max;
   delete [] rand_bw_val;
}

void OfflineScreen::paintEvent( QPaintEvent *)
{
   int x = ( width()  - image_w) >> 1;
   int y = ( height() - image_h) >> 1;
   int rand_num;

   QPainter painter( this);

   // Copy image and add grain:
   static int grain_phase = 0;
   grain_phase ++; if( grain_phase >= image_s) grain_phase = 0; rand_num = rand_all_size[grain_phase];
   for( int i = 0; i < image_s; i++)
   {
      rand_num++;
      if( rand_num >= image_s ) rand_num -= image_s;
      RGBA src, noise, dest;
      src.all = image[i];
      noise.all = rand_all_grain[rand_num];
      for( int c = 0; c < 4; c ++)
      {
         if( c == A ) continue;
         int val = src.c[c];
         val = val * noise.c[c] / 100 + noise.c[c] / 10;
         if( val < 0 ) val = 0;
         if( val > 0xff ) val = 0xff;
         dest.c[c] = val;
      }
      buffer_a[i] = dest.all;
//      buffer_a[i] = src.all; // - disable
   }

   // Offset image:
   static int offset_phase = 0;
   offset_phase ++; if( offset_phase >= image_s) offset_phase = 0;
   rand_num = rand_all[offset_phase];
   for( int i = 0; i < image_s; i++)
   {
      int dest = i + rand_all_size[offset_phase];
      dest = int( dest + float(rand_num) * float(dest) / float(RAND_MAX));
      dest = dest % image_s;
      buffer_b[i] = buffer_a[dest];
//      buffer_b[i] = buffer_a[i]; // - disable
   }

   // Mix black & white:
   static int bw_phase_pos = 0;
   static int bw_phase_val = 0;
   for( int bw = 0; bw < bwsize; bw++)
   {
      bw_phase_pos ++;
      if( bw_phase_pos >= image_s) bw_phase_pos = 0;
      rand_num = rand_all_size[bw_phase_pos];
      for( int i = 0; i < rand_bw_len[bw]; i++)
      {
         int pos = rand_num + rand_bw_pos[bw][i];
         pos = pos % image_s;
         int val = rand_bw_val[bw_phase_val];
         buffer_b[pos] = 0xff000000 + (val<<16) + (val<<8) + val; // - disable
         bw_phase_val++;
         if( bw_phase_val >= image_s) bw_phase_val = 0;
      }
   }

   QImage qimage( (unsigned char*)buffer_b, image_w, image_h, QImage::Format_RGB32);

   painter.drawImage( QRect( x, y, image_w, image_h), qimage);

   if( false == offlineimage.isNull())
   {
      int ox = (rand_all_size[grain_phase>>1] << 1) / image_s;
      int oy = (rand_all_size[grain_phase   ] << 2) / image_s;
      int iw = offlineimage.width();
      int ih = offlineimage.height();
      int ix = x + (image_w >> 1) - (iw >> 1) + ox;
      int iy = y + (image_h >> 1) - (ih >> 1) + oy;
      painter.drawImage( QRect( ix, iy, iw, ih), offlineimage);
   }
}
