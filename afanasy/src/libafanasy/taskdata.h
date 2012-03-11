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

   inline const std::string & getName()        const { return name;       }
   inline const std::string & getCommand()     const { return command;    }
   inline const std::string & getFiles()       const { return files;      }
   inline const std::string & getDependMask()  const { return dependmask; }
   inline const std::string & getCustomData()  const { return customdata; }

   inline void setFiles(      const std::string & str) { files      = str;  }
   inline void setDependMask( const std::string & str) { dependmask = str;  }
   inline void setCustomData( const std::string & str) { customdata = str;  }

   inline bool hasFiles()        const { return      files.empty() == false;}  ///< Whether files are set.
   inline bool hasDependMask()   const { return dependmask.empty() == false;}  ///< Whether depend mask is set.
   inline bool hasCustomData()   const { return customdata.empty() == false;}  ///< Whether files are set.

   bool checkDependMask( const std::string & str);

protected:
   std::string name;        ///< Task name.
   std::string command;     ///< Command.
   std::string files;       ///< Files.
   std::string dependmask;  ///< Dependences.
   std::string customdata;  ///< Some custom data.

protected:
/// Read or write task data.
   virtual void readwrite( Msg * msg);
};
}
