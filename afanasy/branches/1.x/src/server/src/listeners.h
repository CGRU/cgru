#pragma once

#include <address.h>
#include <taskexec.h>
#include <msgclasses/mclistenaddress.h>

/// Afanasy tasks listeners.
class Listeners
{
public:
   Listeners();
   ~Listeners();

   bool process( af::MCListenAddress & mclass);

   void process( af::TaskExec & task) const;

   int calcWeight() const;

private:
   std::list<af::Address*> job_addresses;

   std::list<af::Address*> task_addresses;
   std::list<int> task_blocks;
   std::list<int> task_numbers;
};
