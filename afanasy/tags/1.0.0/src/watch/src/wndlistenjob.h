#pragma once

#include "wndtext.h"
#include "reciever.h"

#include <name_af.h>

class WndListenJob : public WndText, public Reciever
{
public:
   WndListenJob( int JobId, const QString & Name);
   ~WndListenJob();

   bool caseMessage( af::Msg * msg);

   void connectionLost();

protected:
   void closeEvent( QCloseEvent * event);

private:
   int jobid;
   QString jobname;
};
