#include "farm.h"

#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtCore/QRegExp>

//#define AFOUTPUT
////#undef AFOUTPUT
#include <macrooutput.h>

using namespace af;

const QString XMLNAME_PATTERN       = "pattern";
const QString XMLNAME_PATTERNNAME   = "name";
const QString XMLNAME_DESCRIPTION   = "description";
const QString XMLNAME_MASK          = "mask";
const QString XMLNAME_OS            = "os";
const QString XMLNAME_PROPERTIES    = "properties";
const QString XMLNAME_CAPACITY      = "capacity";
const QString XMLNAME_MAXTASKS      = "maxtasks";
const QString XMLNAME_POWER         = "power";
const QString XMLNAME_SERVISE       = "service";
const QString XMLNAME_SERVISENAME   = "name";
const QString XMLNAME_SERVISECOUNT  = "count";

Farm::Farm( const QString & File, bool Verbose ):
   count( 0),
   filename( File),
   ptr_first( NULL),
   ptr_last( NULL),
   valid( false)
{
   QDomDocument doc("farm");

   QFile file( filename);
   if( file.open(QIODevice::ReadOnly) == false)
   {
//      if( Verbose) printf("Unable to open \"%s\".\n", filename.toUtf8().data());
      printf("Unable to open \"%s\".\n", filename.toUtf8().data());
      return;
   }
   QString errorMsg; int errorLine = 0; int errorColumn = 0;
   if( doc.setContent( &file, &errorMsg, &errorLine, &errorColumn) == false)
   {
      AFERRAR("Parse error \"%s\" [Line %d - Col %d]:\n", filename.toUtf8().data(), errorLine, errorColumn);
      printf("%s\n", errorMsg.toUtf8().data());
      file.close();
      return;
   }
   file.close();
   if( Verbose) printf("\nParsing XML file \"%s\":\n", filename.toUtf8().data());

   QDomElement docElem = doc.documentElement();
   QDomNode p = docElem.firstChild();
   while( !p.isNull())
   {
      QDomElement pattern = p.toElement();
      if( !pattern.isNull())
      {
         if( pattern.tagName() == XMLNAME_PATTERN)
         {
            QString name = pattern.attribute( XMLNAME_PATTERNNAME, "");
            QString description = "";
            QString mask;
            Host host;
            if( name == "")
            {
               AFERRAR("Pattern has no name [Line %d - Col %d]: \"%s\"\n",
                  pattern.lineNumber(), pattern.columnNumber(), pattern.tagName().toUtf8().data());
               return;
            }
            if( Verbose) printf("name: %s\n", name.toUtf8().data());
            QDomNode f = pattern.firstChild();
            while( !f.isNull())
            {
               QDomElement field = f.toElement();
               if( !field.isNull())
               {
                  QString fieldname = field.tagName();
                  if( fieldname == XMLNAME_DESCRIPTION)
                  {
                     description = field.text().toUtf8().data();
                     if( Verbose) printf("\tdesc: %s\n", field.text().toUtf8().data());
                  }
                  else if( fieldname == XMLNAME_OS)
                  {
                     host.os = field.text().toUtf8().data();
                     if( Verbose) printf("\tos: \"%s\"\n", field.text().toUtf8().data());
                  }
                  else if( fieldname == XMLNAME_PROPERTIES)
                  {
                     host.properties = field.text().toUtf8().data();
                     if( Verbose) printf("\tproperties: \"%s\"\n", field.text().toUtf8().data());
                  }
                  else if( fieldname == XMLNAME_CAPACITY)
                  {
                     QString capacity_str = field.text().toUtf8().data();
                     int capacity = 0;
                     if( capacity_str != "")
                     {
                        bool ok;
                        capacity = capacity_str.toInt( &ok);
                        if( !ok)
                        {
                           AFERRAR("Invalid machine capacity [Line %d - Col %d]: \"%s\"\n",
                              field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                           return;
                        }
                     }
                     host.capacity = capacity;
                     if( Verbose) printf("\tcapacity = %d\n", host.capacity);
                  }
                  else if( fieldname == XMLNAME_MAXTASKS)
                  {
                     QString maxtasks_str = field.text().toUtf8().data();
                     int maxtasks = 0;
                     if( maxtasks_str != "")
                     {
                        bool ok;
                        maxtasks = maxtasks_str.toInt( &ok);
                        if( !ok)
                        {
                           AFERRAR("Invalid machine maxtasks [Line %d - Col %d]: \"%s\"\n",
                              field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                           return;
                        }
                     }
                     host.maxtasks = maxtasks;
                     if( Verbose) printf("\tmaxtasks = %d\n", host.maxtasks);
                  }
                  else if( fieldname == XMLNAME_POWER)
                  {
                     QString power_str = field.text().toUtf8().data();
                     int power = 0;
                     if( power_str != "")
                     {
                        bool ok;
                        power = power_str.toInt( &ok);
                        if( !ok)
                        {
                           AFERRAR("Invalid machine power [Line %d - Col %d]: \"%s\"\n",
                              field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                           return;
                        }
                     }
                     host.power = power;
                     if( Verbose) printf("\tpower = %d\n", host.power);
                  }
                  else if( fieldname == XMLNAME_MASK)
                  {
                     mask = field.text().toUtf8().data();
                     QRegExp regexp( mask);
                     if( regexp.isValid() == false)
                     {
                        AFERRAR("Pattern::setMask: \"%s\" set invalid mask \"%s\":\n",
                           name.toUtf8().data(), mask.toUtf8().data());
                        printf("%s\n", regexp.errorString().toUtf8().data());
                        printf("[Line %d - Col %d]: \"%s\"\n",
                           field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                        return;
                     }
                     if( Verbose) printf("\tmask: %s\n", field.text().toUtf8().data());
                  }
                  else if( fieldname == XMLNAME_SERVISE)
                  {
                     QString servicename = field.attribute( XMLNAME_SERVISENAME, "");
                     int servicecount = 0;
                     if( servicename == "")
                     {
                        AFERRAR("Service has no name [Line %d - Col %d]: \"%s\"\n",
                           field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                        return;
                     }
                     QString servicecount_str = field.attribute( XMLNAME_SERVISECOUNT, "");
                     if( servicecount_str != "")
                     {
                        bool ok;
                        servicecount = servicecount_str.toInt( &ok);
                        if( !ok)
                        {
                           AFERRAR("Service has invalid count [Line %d - Col %d]: \"%s\"\n",
                              field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                           return;
                        }
                     }
                     host.setService( servicename, servicecount);
                     if( Verbose) printf("\t%s: %d\n", servicename.toUtf8().data(), servicecount);
                  }
               }
               f = f.nextSibling();
            }
            Pattern * p = new Pattern( name);
            p->setMask( mask);
            p->setDescription( description);
            p->setHost( host);
            if( addPattern(p) == false)
            {
               delete p;
               return;
            }
         }
         else
         {
            printf("Unknown element [Line %d - Col %d]: \"%s\"\n",
               pattern.lineNumber(), pattern.columnNumber(), pattern.tagName().toUtf8().data());
         }
      }
      p = p.nextSibling();
   }
   if( count) valid = true;
   else
      AFERRAR("No patterns founded in \"%s\"\n", filename.toUtf8().data());
}

Farm::~Farm()
{
   while( ptr_first != NULL)
   {
      ptr_last = ptr_first;
      ptr_first = ptr_first->ptr_next;
      delete ptr_last;
   }
}

bool Farm::addPattern( Pattern * p)
{
   if( p->isValid() == false)
   {
      AFERRAR("Farm::addPattern: invalid pattern \"%s\"\n", p->getName().toUtf8().data());
      return false;
   }
   if( ptr_first == NULL)
   {
      ptr_first = p;
   }
   else
   {
      ptr_last->ptr_next = p;
   }
   ptr_last = p;
   count++;
   return true;
}

void Farm::stdOut( bool full) const
{
   if( full) printf("\n");
   printf("Farm filename = \"%s\":\n", filename.toUtf8().data());
//   if( full) printf("\n");
   Pattern * p = ptr_first;
   while( p != NULL)
   {
      p->stdOut(full);
//      if( full) printf("\n");
      p = p->ptr_next;
   }
}

bool Farm::getHost( const QString & hostname, Host & host) const
{
   Pattern * ptr = NULL;
   for( Pattern * p = ptr_first; p != NULL; p = p->ptr_next) if( p->match( hostname)) ptr = p;
   if( ptr == NULL) return false;
   ptr->getHost( host);
   return true;
}

bool Farm::getHost( const QString & hostname, Host & host, QString & name) const
{
   Pattern * ptr = NULL;
   for( Pattern * p = ptr_first; p != NULL; p = p->ptr_next) if( p->match( hostname)) ptr = p;
   if( ptr == NULL) return false;
   name = ptr->getName();
   return true;
}

bool Farm::getHost( const QString & hostname, Host & host, QString & name, QString & description) const
{
   Pattern * ptr = NULL;
   for( Pattern * p = ptr_first; p != NULL; p = p->ptr_next) if( p->match( hostname)) ptr = p;
   if( ptr == NULL) return false;
   ptr->getHost( host);
   name = ptr->getName();
   description = ptr->getDescription();
   return true;
}
