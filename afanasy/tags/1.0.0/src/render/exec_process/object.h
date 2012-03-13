#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "process.h"

#include <QWidget>

class MyObject : public QWidget
{
Q_OBJECT
public:
   MyObject( int argc, char ** argv);
   ~MyObject();
private:
   QString program;
   QStringList arguments;
   int task_type;
   char* output_str;
   int frames;

   MyProcess *myProcess;

private slots:
   void function();
   void kill_function();
};
