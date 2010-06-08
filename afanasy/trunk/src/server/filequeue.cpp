#include "filequeue.h"

#include <QtCore/QFile>

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

FileQueue::FileQueue( const QString & QueueName):
   AfQueue( QueueName)
{
}

FileQueue::~FileQueue()
{
}

void FileQueue::processItem( AfQueueItem* item) const
{
   FileData * filedata = (FileData*)item;
   AFINFA("FileQueue::processItem: \"%s\"\n", filedata->getFileName().toUtf8().data());
   int rotate = filedata->getRotate();
   QString filename( filedata->getFileName());
   if( rotate > 0)
   {
#ifdef AFOUTPUT
printf("FileQueue::processItem: rotating \"%s\" %d times:\n", filename.toUtf8().data(), rotate);
#endif
      if( QFile::exists(filename))
      {
         renameNext( filename, 0, rotate);
         QFile::rename( filename, QString("%1.0").arg(filename));
      }
   }
   else if( rotate == -1)
   {
      QString timedel = QDateTime::currentDateTime().toString("yyMMdd_hhmm_ss_zzz");
      filename = QString("%1.%2").arg( filename, timedel);
   }
   AFCommon::writeFile( filedata->getData(), filedata->getLength(), filename);
}

void FileQueue::renameNext( const QString & filename, int number, int maxnumber) const
{
   QString curname( QString("%1.%2").arg(filename).arg(number++));
   if( number >= maxnumber )
   {
#ifdef AFOUTPUT
printf("FileQueue::renameNext: removing \"%s\"\n", curname.toUtf8().data());
#endif
      if( QFile::remove( curname) == false)
         AFERRAR("FileQueue::renameNext: unable to remove: \"%s\"\n", curname.toUtf8().data());
      return;
   }
   if( QFile::exists( curname) == false) return;

   QString newname( QString("%1.%2").arg(filename).arg(number));

   renameNext( filename, number, maxnumber);

#ifdef AFOUTPUT
printf("FileQueue::renameNext: renaming \"%s\" in \"%s\"\n", curname.toUtf8().data(), newname.toUtf8().data());
#endif
   if( QFile::rename( curname, newname) == false)
      AFERRAR("FileQueue::renameNext: unable to rename: \"%s\" in \"%s\"\n",
         curname.toUtf8().data(), newname.toUtf8().data());
}
