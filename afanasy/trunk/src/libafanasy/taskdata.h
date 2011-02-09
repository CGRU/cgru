#pragma once

#include "af.h"

namespace af
{
class TaskData : public Af
{
public:

   TaskData();

   TaskData( Msg * msg);

   virtual ~TaskData();

   int calcWeight() const;                         ///< Calculate and return memory size.
   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   inline const QString& getName()        const { return name;       }
   inline const QString& getCommand()     const { return command;    }
   inline const QString& getFiles()       const { return files;      }
   inline const QString& getDependMask()  const { return dependmask; }
   inline const QString& getCustomData()  const { return customdata; }

   inline void setFiles(      const QString & str) { files      = str;  }
   inline void setDependMask( const QString & str) { dependmask = str;  }
   inline void setCustomData( const QString & str) { customdata = str;  }

   inline bool hasFiles()        const { return      !files.isEmpty();}  ///< Whether files are set.
   inline bool hasDependMask()   const { return !dependmask.isEmpty();}  ///< Whether depend mask is set.
   inline bool hasCustomData()   const { return !customdata.isEmpty();}  ///< Whether files are set.

   bool checkDependMask( const QString & str);

protected:
   QString name;        ///< Task name.
   QString command;     ///< Command.
   QString files;       ///< Files.
   QString dependmask;  ///< Dependences.
   QString customdata;  ///< Some custom data.

protected:
/// Read or write task data.
   virtual void readwrite( Msg * msg);
};
}
