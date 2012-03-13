#pragma once

#include "aflist.h"
#include "aflistit.h"
#include "jobaf.h"

/// List to store some jobs, for some user for example.
class JobsList : public AfList
{
public:
   JobsList();
   ~JobsList();

   void addJob( JobAf* job);

protected:

private:
};

//########################## Iterator ##############################

/// JobsList iterator.
class JobsListIt : public AfListIt
{
public:
   JobsListIt( JobsList* jobslist);
   ~JobsListIt();

   inline JobAf* job() { return (JobAf*)node; }

private:
};
