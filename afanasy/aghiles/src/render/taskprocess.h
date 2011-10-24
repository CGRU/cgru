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
   TaskProcess( QObject * parent, af::TaskExec * taskExec, int runningtasks);
   ~TaskProcess();

   inline bool is( int jobId, int blockNum, int taskNum, int Number) const
      { return ((exec->getJobId() == jobId) && (exec->getBlockNum() == blockNum) && (exec->getTaskNum() == taskNum) && (exec->getNumber() == Number));}

   inline bool is( const af::MCTaskPos & taskpos) const
      { return is( taskpos.getJobId(), taskpos.getNumBlock(), taskpos.getNumTask(), taskpos.getNumber());}

   void getOutput( af::Msg & msg) const;

   void stop( bool noStatusUpdate = false);

   void refresh();

   inline bool isZombie() const { return zombie;}

public:
   af::TaskExec * exec;

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
