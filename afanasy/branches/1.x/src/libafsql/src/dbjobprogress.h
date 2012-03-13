#pragma once

#include <name_af.h>
#include <jobprogress.h>

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

   void dbAdd( QSqlDatabase * db) const;

   bool dbSelect( QSqlDatabase * db);

private:
   virtual af::TaskProgress * newTaskProgress() const;

private:
};
}
