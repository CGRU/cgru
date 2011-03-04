#include "mcjobsweight.h"

#include <stdio.h>

#include "../name_af.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCJobsWeight::MCJobsWeight()
{
}

MCJobsWeight::MCJobsWeight( Msg * msg)
{
   read( msg);
}

MCJobsWeight::~MCJobsWeight()
{
}

void MCJobsWeight::add(
         const std::string & Name,
         int Id,
         int WeightLog,
         int WeightBlack,
         int WeightProgress,
         int Weight)
{
   names.push_back( Name);
   ids.push_back( Id);
   weightLog.push_back( WeightLog);
   weightBlack.push_back( WeightBlack);
   weightProgress.push_back( WeightProgress);
   weight.push_back( Weight);
}

void MCJobsWeight::readwrite( Msg * msg)
{
   rw_StringVect( names,           msg);
   rw_Int32_Vect( ids,             msg);
   rw_Int32_Vect( weightLog,       msg);
   rw_Int32_Vect( weightBlack,     msg);
   rw_Int32_Vect( weightProgress,  msg);
   rw_Int32_Vect( weight,          msg);
}

void MCJobsWeight::stdOut( bool full) const
{
   printf("Jobs memory: ");
   if( full ) printf("\n");

   int weightLogTotal = 0;
   int weightBlackTotal = 0;
   int weightProgressTotal = 0;
   int weightTotal = 0;
   for( unsigned j = 0; j < ids.size(); j++)
   {
      weightLogTotal       += weightLog[j];
      weightBlackTotal     += weightBlack[j];
      weightProgressTotal  += weightProgress[j];
      weightTotal          += weight[j];
      if( full )
         printf( "%d-%d: \tlog=%.3g \tblack=%.3g \tprogress=%.3g \t : %.3g kB\t - %s\n",
                 j+1, ids[j], float(weightLog[j])/1024, float(weightBlack[j])/1024, float(weightProgress[j])/1024, float(weight[j])/1024, names[j].c_str());
   }
   printf("Logs = %.3g, Black Lists = %.3g, Tasks Progress = %.3g, Total Jobs Weight = %.3g MB\n",
           float(weightLogTotal)/(1024*1024), float(weightBlackTotal)/(1024*1024), float(weightProgressTotal)/(1024*1024), float(weightTotal)/(1024*1024));
}
