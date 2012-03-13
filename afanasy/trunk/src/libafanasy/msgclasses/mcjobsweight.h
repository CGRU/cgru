#pragma once

#include <stdlib.h>

#include "../../include/aftypes.h"

#include <QtCore/QStringList>

#include "msgclass.h"

namespace af
{

class MCJobsWeight : public MsgClass
{
public:
   MCJobsWeight();
   MCJobsWeight( Msg * msg);
   ~MCJobsWeight();

   void add(
         const QString & Name,
         int Id,
         int WeightLog,
         int WeightBlack,
         int WeightProgress,
         int Weight);

   void stdOut( bool full = false) const;

   inline int getCount()                       const { return ids.size(); }
   inline int getId(             unsigned pos) const { if(pos<ids.size())return ids[pos];            else return -1; }
   inline int getWeightLog(      unsigned pos) const { if(pos<ids.size())return weightLog[pos];      else return -1; }
   inline int getWeightBlack(    unsigned pos) const { if(pos<ids.size())return weightBlack[pos];    else return -1; }
   inline int getWeightProgress( unsigned pos) const { if(pos<ids.size())return weightProgress[pos]; else return -1; }
   inline int getWeight(         unsigned pos) const { if(pos<ids.size())return weight[pos];         else return -1; }
   inline const QString getName( unsigned pos) const { if(pos<ids.size())return names[pos];          else return ""; }

private:
   QStringList names;
   std::vector<int32_t> ids;
   std::vector<int32_t> weightLog;
   std::vector<int32_t> weightBlack;
   std::vector<int32_t> weightProgress;
   std::vector<int32_t> weight;

   void readwrite( Msg * msg);
};
}
