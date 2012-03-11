#pragma once

#include "../libafanasy/msgqueue.h"

class FileData: public af::AfQueueItem
{
public:

/// Rotate =  0: no rotation
/** Rotate = -1: add time to name
**/
   FileData( const char * Data, int Length, const std::string & FileName, int Rotate = 0);
   ~FileData();

   inline const char *        getData()     const { return data;     }
   inline const std::string & getFileName() const { return filename; }

   inline int getLength() const { return length; }
   inline int getRotate() const { return rotate; }

private:
   std::string filename;
   int length;
   int rotate;
   char * data;
};
