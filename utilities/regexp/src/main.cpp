#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtCore/QDir>

#include "dialog.h"

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   QString iconsdir = "icons";
   QString iconname = "regexp.svg";
   QIcon * icon = NULL;
   QDir dir( argv[0]);
   dir.makeAbsolute();
   dir.cdUp();
   dir.cdUp();
   dir.cd( iconsdir);
   if( dir.exists( iconname)) icon = new QIcon( dir.filePath( iconname));

   Dialog dialog;
   if( icon ) dialog.setWindowIcon( *icon);
   dialog.show();
   if( icon ) delete icon;

   return app.exec();
}
