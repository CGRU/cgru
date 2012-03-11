#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/jobprogress.h"

#include "name_afsql.h"

namespace afsql
{
/// Job progress information class.
class DBJobProgress : public af::JobProgress
{
public:
/// Construct progress for given job.
   DBJobProgress( af::Job * job);

   virtual ~DBJobProgress();

   bool dbAdd( PGconn * i_conn) const;

   bool dbSelect( PGconn * i_conn);

private:
   virtual af::TaskProgress * newTaskProgress() const;

private:
};
}
