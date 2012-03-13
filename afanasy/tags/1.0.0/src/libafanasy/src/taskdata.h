#ifndef AF_TASKDATA_H
#define AF_TASKDATA_H

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
   void stdOut( bool full) const;

   inline const QString& getName()       const { return name;        }
   inline const QString& getCmd()        const { return cmd;         }
   inline const QString& getCmdView()    const { return cmd_view;    }
   inline const QString& getDependMask() const { return dependmask;  }

   inline void setCmdView( const QString & str) { cmd_view = str;}
   inline void setDepend(  const QString & str) { dependmask = str;}

   inline bool hasCmdView()    const { return   !cmd_view.isEmpty();}  ///< Whether view command is set.
   inline bool hasDependMask() const { return !dependmask.isEmpty();}  ///< Whether depend mask is set.

   bool checkDependMask( const QString & str);

protected:
   QString name;        ///< Task name.
   QString cmd;         ///< Command.
   QString cmd_view;    ///< Command to view task result.
   QString dependmask;  ///< Dependences.

protected:
/// Read or write task data.
   virtual void readwrite( Msg * msg);
};
}
#endif
