#pragma once

#include "wndtext.h"
#include "reciever.h"

#include <name_af.h>

class WndListenTask : public WndText, public Reciever
{
public:
   WndListenTask( int JobId, int BlockNum, int TaskNum, const QString & Name);
   ~WndListenTask();

   bool caseMessage( af::Msg * msg);

   void connectionLost();

protected:
   void closeEvent( QCloseEvent * event);

private:
   int jobid;
   int block;
   int task;
   QString taskname;
};
