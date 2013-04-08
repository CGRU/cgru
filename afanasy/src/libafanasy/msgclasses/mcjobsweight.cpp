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

void MCJobsWeight::v_readwrite( Msg * msg)
{
   rw_StringVect( names,           msg);
   rw_Int32_Vect( ids,             msg);
   rw_Int32_Vect( weightLog,       msg);
   rw_Int32_Vect( weightBlack,     msg);
   rw_Int32_Vect( weightProgress,  msg);
   rw_Int32_Vect( weight,          msg);
}

void MCJobsWeight::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "Jobs memory: ";
   if( full ) stream << std::endl;

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
         stream << "\n" << j+1 << ids[j]
               << ": \tlog=" << float(weightLog[j])/1024
               << " \tblack=" << float(weightBlack[j])/1024
               << " \tprogress=" << float(weightProgress[j])/1024
               << " \t : " << float(weight[j])/1024 << " kB"
               << " \t - " << names[j];
   }
   stream << "\nLogs = " << float(weightLogTotal)/(1024*1024)
         << ", Black Lists = " << float(weightBlackTotal)/(1024*1024)
         << ", Tasks Progress = " << float(weightProgressTotal)/(1024*1024)
         << ", Total Jobs Weight = " << float(weightTotal)/(1024*1024) << " MB";
}
