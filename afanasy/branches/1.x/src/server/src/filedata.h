#pragma once

#include <QtCore/QString>

#include "afqueueitem.h"

class FileData: public AfQueueItem
{
public:

   FileData( char * Data, int Length, const QString & FileName, int Rotate = 0);
   ~FileData();

   inline const char * getData()     const { return data;     }
   inline const char * getFileName() const { return filename.toUtf8().data(); }

   inline int getLength() const { return length; }
   inline int getRotate() const { return rotate; }

private:
   QString filename;
   int length;
   int rotate;
   char * data;
};
