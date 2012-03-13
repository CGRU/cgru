#ifndef PROCESS_H
#define PROCESS_H

#include <QProcess>
#include <QTimer>

#include "../src/parser.h"

#define BUFFER_SIZE 1024

class MyProcess : public QProcess
{
Q_OBJECT
public:
   MyProcess( QObject * parent, int tasktype, char *outputstr, int numframes);
   ~MyProcess();

   void resetOpenMode( QIODevice::OpenMode mode);
   void begin( const QString& program, const QStringList& arguments);

private:

   QTimer timer;
   char data_buffer[ BUFFER_SIZE];
   Parser *parser;
   char* output_str;
   int task_type;
   int frames;

protected:
   virtual void setupChildProcess();

public slots:
   void kill();
   void terminate();

private slots:
// QProcess signals:
   void s_started();
   void s_finished( int exitCode, QProcess::ExitStatus exitStatus);
   void s_error( QProcess::ProcessError error);
   void s_readyReadStandardError();
   void s_readyReadStandardOutput();

// QIODevice signals:
   void s_readyRead();
};

#endif
