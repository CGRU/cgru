#pragma once

#include <stdint.h>

#include "../libafanasy/environment.h"

#include <QtGui/QWidget>

class OfflineScreen : public QWidget
{
public:
   OfflineScreen( QWidget * widget);
   ~OfflineScreen();

protected:
   virtual void paintEvent( QPaintEvent *);

private:
   QTimer * timer;

   uint32_t * image;
   uint32_t * buffer_a;
   uint32_t * buffer_b;

   int image_w;
   int image_h;
   int image_s;

   int * rand_all;
   int * rand_all_size;
   int * rand_all_grain;
   int bwsize;
   int * rand_bw_len;
   int * rand_bw_max;
   int ** rand_bw_pos;
   int * rand_bw_val;

   QImage offlineimage;
};
