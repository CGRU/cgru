#pragma once

#include <QtCore/QProcess>

#ifndef Q_OS_UNIX
#define WINVER 0x0500
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

#if defined Q_OS_UNIX
protected:
   virtual void setupChildProcess();
#else
public:
   void setupChildProcess();
private:
   HANDLE hJob;
#endif //Q_OS_UNIX
};
