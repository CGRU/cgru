#pragma once

#include <QtCore/QTimer>
#include <QtCore/QProcess>

#include "../libafanasy/name_af.h"
#include "../libafanasy/service.h"
#include "../libafanasy/taskexec.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "../libafqt/name_afqt.h"

#include "childprocess.h"

class ParserHost;
class QObjectRender;

class TaskProcess : public ChildProcess
{
   Q_OBJECT

public:
   TaskProcess( af::TaskExec * i_taskExec);
   ~TaskProcess();

   inline bool is( int i_jobId, int i_blockNum, int i_taskNum, int i_Number) const
      { return ((m_taskexec->getJobId() == i_jobId) &&
                (m_taskexec->getBlockNum() == i_blockNum) &&
                (m_taskexec->getTaskNum() == i_taskNum) &&
                (m_taskexec->getNumber() == i_Number));}

   inline bool is( const af::MCTaskPos & i_taskpos) const
      { return is( i_taskpos.getJobId(), i_taskpos.getNumBlock(), i_taskpos.getNumTask(), i_taskpos.getNumber());}

   void getOutput( af::Msg & msg) const;

   void stop( bool noStatusUpdate = false);

   void refresh();

   inline bool isZombie() const { return zombie;}

public:
   af::TaskExec * m_taskexec;

private slots:
   void sendTaskSate();
   void killProcess();
   void p_finished( int exitCode, QProcess::ExitStatus exitStatus);
   void p_readyRead();

private:
   af::Service service;
   ParserHost * parser;
   QTimer timer;
   uint8_t update_status;
   time_t stop_time;
   bool zombie;
};
