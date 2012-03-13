#ifndef MCGENERAL_H
#define MCGENERAL_H

#include <aftypes.h>
#include <stdlib.h>
#include <vector>

#include <QtCore/QString>

#include "msgclassuserhost.h"

namespace af
{

class MCGeneral : public MsgClassUserHost
{
public:
   MCGeneral();
   MCGeneral( const QString & String);
   MCGeneral( int Number);
   MCGeneral( const QString & Name, int Number);
   MCGeneral( const QString & Name, const QString & String);

   MCGeneral( Msg * msg);
   ~MCGeneral();

   inline void setNumber( int Number) { number = Number;}
   inline void setString( const QString & String) { string = String;}

   inline void addId( int ID) { list.push_back( ID);}
   inline void setId( int ID) { id = ID;}

   void stdOut( bool full = false) const;

   inline const QString & getName()   const { return name;   }
   inline const QString & getString() const { return string; }

   inline int getCount() const { return list.size(); }
   inline int getId( unsigned pos) const { if(pos<list.size())return list[pos]; else return -1; }
   inline int getId() const { return id; }
   bool hasId( int value);

   inline int getNumber() const { return number; }

   inline const std::vector<int32_t> * getList() const { return &list; }

   inline void clearIds() { list.clear(); }

private:
   QString  name;
   QString  string;
   int32_t number;
   int32_t id;

   std::vector<int32_t> list;

   void readwrite( Msg * msg);
};
}
#endif
