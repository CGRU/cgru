#include "object.h"

#include "../../afjob.h"

#include <QPushButton>
#include <QVBoxLayout>

MyObject::MyObject( int argc, char ** argv)
{
   output_str = new char[strlen(argv[2])+1];
   strcpy( output_str, argv[2]);
   frames = atoi(argv[3]);

   task_type = 0;
   for( uint8_t t = 0; t < AFJOB::TaskTypesQuantity; t++)
   {
      if( strcmp( argv[1], AFJOB::TaskTypeStr[t]) == 0)
      {
         task_type = t;
         break;
      }
      if( t == AFJOB::TaskTypesQuantity-1)
      {
         printf("MyObject::MyObject(): unknown task_type type=\"%s\".\n", argv[1]);
      }
   }

   printf("\nTask type = \"%s\"\n", AFJOB::TaskTypeStr[task_type]);
   printf("\nOutput_str = \"%s\", frames = %d\n", output_str, frames);

   program = argv[4];
   for( int i = 0; i < (argc-5); i++) arguments << argv[i+5];
   printf("command = \"%s", program.toUtf8().data());
   for( int i = 0; i < arguments.size(); i++) printf(" %s", arguments[i].toUtf8().data());
   printf("\"\n");

   QVBoxLayout *vlayout = new QVBoxLayout();

   QPushButton *button = new QPushButton("   E X E C U T E   ",this);
   connect( button, SIGNAL(pressed()), this, SLOT(function()));
   QPushButton *kbutton = new QPushButton("   K I L L   ",this);
   connect( kbutton, SIGNAL(pressed()), this, SLOT(kill_function()));

   vlayout->addWidget( button);
   vlayout->addWidget( kbutton);
   setLayout( vlayout);
}

MyObject::~MyObject()
{
   if( output_str != NULL) delete output_str;
}

void MyObject::function()
{
   myProcess = new MyProcess(this, task_type, output_str, frames);
   myProcess->setProcessChannelMode(QProcess::MergedChannels);
   myProcess->begin( program, arguments);
}

void MyObject::kill_function()
{
   myProcess->terminate();
}
