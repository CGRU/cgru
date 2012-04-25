#pragma once

#include "actionid.h"
#include "listjobs.h"

#include <QtGui/QLabel>

class ListJobs;

class CtrlJobs : public QLabel
{
Q_OBJECT
public:
   CtrlJobs( QWidget * i_parent, ListJobs * i_listjobs);
   ~CtrlJobs();

protected:
   void contextMenuEvent( QContextMenuEvent * i_event);

private:
   ListJobs * m_listjobs;
};
