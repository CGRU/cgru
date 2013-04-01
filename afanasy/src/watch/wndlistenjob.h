#pragma once

#include "../libafanasy/name_af.h"

#include "wndtext.h"
#include "reciever.h"

class WndListenJob : public WndText, public Reciever
{
public:
   WndListenJob( int JobId, const QString & Name);
   ~WndListenJob();

   bool caseMessage( af::Msg * msg);

   void v_connectionLost();

protected:
   void closeEvent( QCloseEvent * event);

private:
   int jobid;
   QString jobname;
};
