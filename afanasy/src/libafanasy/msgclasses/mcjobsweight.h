#pragma once

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
         const std::string & Name,
         int Id,
         int WeightLog,
         int WeightBlack,
         int WeightProgress,
         int Weight);

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

   inline size_t getCount()                        const { return ids.size(); }
   inline int getId(                 unsigned pos) const { if(pos<ids.size())return ids[pos];            else return -1; }
   inline int getWeightLog(          unsigned pos) const { if(pos<ids.size())return weightLog[pos];      else return -1; }
   inline int getWeightBlack(        unsigned pos) const { if(pos<ids.size())return weightBlack[pos];    else return -1; }
   inline int getWeightProgress(     unsigned pos) const { if(pos<ids.size())return weightProgress[pos]; else return -1; }
   inline int getWeight(             unsigned pos) const { if(pos<ids.size())return weight[pos];         else return -1; }
   inline const std::string getName( unsigned pos) const { if(pos<ids.size())return names[pos];          else return ""; }

private:
   std::vector<std::string> names;
   std::vector<int32_t> ids;
   std::vector<int32_t> weightLog;
   std::vector<int32_t> weightBlack;
   std::vector<int32_t> weightProgress;
   std::vector<int32_t> weight;

   void v_readwrite( Msg * msg);
};
}
