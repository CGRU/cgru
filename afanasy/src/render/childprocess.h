#pragma once
#if 0
#include <QtCore/QProcess>

#ifdef WINNT
#include <windows.h>
#endif

class ChildProcess : public QProcess
{
Q_OBJECT
public:
   ChildProcess( QObject * parent = 0);
   ~ChildProcess();

public slots:
   void kill();
   void terminate();

#ifndef WINNT
protected:
   virtual void setupChildProcess();
#else
public:
   void setupChildProcess();
private:
   PROCESS_INFORMATION * process_info;
   HANDLE hJob;
#endif
};
#endif
