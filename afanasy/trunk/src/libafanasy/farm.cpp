#include "farm.h"

#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtCore/QRegExp>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

const QString XMLNAME_PATTERN          = "pattern";
const QString XMLNAME_PATTERNNAME      = "name";
const QString XMLNAME_DESCRIPTION      = "description";
const QString XMLNAME_MASK             = "mask";
const QString XMLNAME_OS               = "os";
const QString XMLNAME_PROPERTIES       = "properties";
const QString XMLNAME_POWER            = "power";
const QString XMLNAME_RESOURCES        = "resources";
const QString XMLNAME_DATA             = "data";
const QString XMLNAME_CAPACITY         = "capacity";
const QString XMLNAME_MAXTASKS         = "maxtasks";
const QString XMLNAME_SERVISE          = "service";
const QString XMLNAME_SERVISEREMOVE    = "remservice";
const QString XMLNAME_SERVISENAME      = "name";
const QString XMLNAME_SERVISECOUNT     = "count";
const QString XMLNAME_SERVISEMAXHOSTS  = "maxhosts";
const QString XMLNAME_SERVISEMAXCOUNT  = "maxcount";

ServiceLimit::ServiceLimit( int MaxCount, int MaxHosts):
   maxcount( MaxCount),
   maxhosts( MaxHosts),
   counter( 0)
{
}


bool ServiceLimit::canRun( const std::string & hostname) const
{
   if( maxcount != -1 ) if( counter >= maxcount ) return false; // Check maximum count
   if( maxhosts != -1 ) // Check maximum hosts
   {
      // If host already exists service can run on it:
      for( std::list< std::string>::const_iterator it = hostslist.begin(); it != hostslist.end(); it++)
         if( *it == hostname ) return true;

      // Check whether we can add one more host:
      if( hostslist.size() >= maxhosts ) return false;
   }
   return true;
}

void ServiceLimit::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
      stream << "Count = " << counter << "/" <<  maxcount << "; Hosts = " << hostslist.size() << "/" << maxhosts;
   else
      stream << "c" << counter << "/" <<  maxcount << " h" << hostslist.size() << "/" << maxhosts;
/*
   if( hostslist.size())
   {
      stream << ": ";
      if( full ) stream << std::endl;
      for( std::list< std::string>::const_iterator it = hostslist.begin(); it != hostslist.end(); it++)
         stream << " " << *it;
   }
*/
}

bool ServiceLimit::increment( const std::string & hostname)
{
   bool retval = true;

   if( counter >= maxcount ) retval = false; // Check counter
   counter++;                                // Increase counter

   if( hostslist.size() >= maxhosts ) retval = false; // Check hosts list

   bool hostexists = false;
   for( std::list< std::string>::const_iterator it = hostslist.begin(); it != hostslist.end(); it++)
      if( *it == hostname )
      {
         hostexists = true;
         break;
      }
   if( false == hostexists ) hostslist.push_back( hostname); // Appent hostslist with hostname if it does not exist

   return retval;
}

bool ServiceLimit::releaseHost( const std::string & hostname)
{
   bool retval = true;

   if( counter <= 0 ) retval = false;
   else counter--;

   bool hostexists = false;
   std::list< std::string>::iterator it = hostslist.begin();
   for( ; it != hostslist.end(); it++)
      if( *it == hostname )
      {
         hostexists = true;
         hostslist.erase( it);
         break;
      }
   if( false == hostexists ) retval = false;

   return retval;
}

void ServiceLimit::getLimits( const ServiceLimit & other)
{
   if( other.counter >= 0 ) counter = other.counter;
   hostslist = other.hostslist;
}

//############################################## Farm ########################################

Farm::Farm( const std::string & File, bool Verbose ):
   count( 0),
   filename( File),
   ptr_first( NULL),
   ptr_last( NULL),
   valid( false)
{
   QDomDocument doc("farm");

   QFile file( filename.c_str());
   if( file.open(QIODevice::ReadOnly) == false)
   {
      printf("Unable to open \"%s\".\n", filename.c_str());
      return;
   }
   QString errorMsg; int errorLine = 0; int errorColumn = 0;
   if( doc.setContent( &file, &errorMsg, &errorLine, &errorColumn) == false)
   {
      AFERRAR("Parse error \"%s\" [Line %d - Col %d]:\n", filename.c_str(), errorLine, errorColumn);
      printf("%s\n", errorMsg.toUtf8().data());
      file.close();
      return;
   }
   file.close();
   if( Verbose) printf("\nParsing XML file \"%s\":\n", filename.c_str());

   QDomElement docElem = doc.documentElement();
   QDomNode patDomNode = docElem.firstChild();
   while( !patDomNode.isNull())
   {
      QDomElement topDomElement = patDomNode.toElement();
      if( !topDomElement.isNull())
      {
         if( topDomElement.tagName() == XMLNAME_PATTERN)
         {
            std::string name = topDomElement.attribute( XMLNAME_PATTERNNAME, "").toUtf8().data();
            std::string description = "";
            std::string mask;
            Host host;
            std::list<std::string> remservices;
            if( name == "")
            {
               AFERRAR("Pattern has no name [Line %d - Col %d]: \"%s\"\n",
                  topDomElement.lineNumber(), topDomElement.columnNumber(), topDomElement.tagName().toUtf8().data());
               return;
            }
            if( Verbose) printf("name: %s\n", name.c_str());
            QDomNode f = topDomElement.firstChild();
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
                  else if( fieldname == XMLNAME_RESOURCES)
                  {
                     host.resources = field.text().toUtf8().data();
                     if( Verbose) printf("\tresources: \"%s\"\n", field.text().toUtf8().data());
                  }
                  else if( fieldname == XMLNAME_DATA)
                  {
                     host.data= field.text().toUtf8().data();
                     if( Verbose) printf("\tdata: \"%s\"\n", field.text().toUtf8().data());
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
                     QRegExp regexp( QString::fromUtf8( mask.c_str()));
                     if( regexp.isValid() == false)
                     {
                        AFERRAR("Pattern::setMask: \"%s\" set invalid mask \"%s\":\n",
                           name.c_str(), mask.c_str());
                        printf("%s\n", regexp.errorString().toUtf8().data());
                        printf("[Line %d - Col %d]: \"%s\"\n",
                           field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                        return;
                     }
                     if( Verbose) printf("\tmask: %s\n", field.text().toUtf8().data());
                  }
                  else if( fieldname == XMLNAME_SERVISE)
                  {
                     std::string servicename = field.attribute( XMLNAME_SERVISENAME, "").toUtf8().data();
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
                     if( Verbose) printf("\t%s: %d\n", servicename.c_str(), servicecount);
                  }
                  else if( fieldname == XMLNAME_SERVISEREMOVE)
                  {
                     std::string servicename = field.attribute( XMLNAME_SERVISENAME, "").toUtf8().data();
                     if( servicename.size() == 0 )
                     {
                        AFERRAR("Service to delete has no name [Line %d - Col %d]: \"%s\"\n",
                           field.lineNumber(), field.columnNumber(), field.tagName().toUtf8().data());
                        return;
                     }
                     if( Verbose) printf("\tRemove Service \"%s\"", servicename.c_str());
                     remservices.push_back( servicename);
                  }
               }
               f = f.nextSibling();
            }
            Pattern * pat = new Pattern( name);
            pat->setMask( mask);
            pat->setDescription( description);
            pat->setHost( host);
            pat->remServices( remservices);
            if( addPattern( pat) == false)
            {
               delete pat;
               return;
            }
         }
         else if( topDomElement.tagName() == XMLNAME_SERVISE)
         {
            QString servicename = topDomElement.attribute( XMLNAME_SERVISENAME, "");
            if( servicename == "")
            {
               AFERRAR("Service has no name [Line %d - Col %d]: \"%s\"\n",
                  topDomElement.lineNumber(), topDomElement.columnNumber(), topDomElement.tagName().toUtf8().data());
               return;
            }
            int servicemaxcount = -1;
            int servicemaxhosts = -1;
            QString servicemaxcount_str = topDomElement.attribute( XMLNAME_SERVISEMAXCOUNT, "");
            if( servicemaxcount_str != "")
            {
               bool ok;
               servicemaxcount = servicemaxcount_str.toInt( &ok);
               if( !ok)
               {
                  AFERRAR("Service has invalid maximum total count [Line %d - Col %d]: \"%s\"\n",
                     topDomElement.lineNumber(), topDomElement.columnNumber(), topDomElement.tagName().toUtf8().data());
                  return;
               }
            }
            QString servicemaxhosts_str = topDomElement.attribute( XMLNAME_SERVISEMAXHOSTS, "");
            if( servicemaxhosts_str != "")
            {
               bool ok;
               servicemaxhosts = servicemaxhosts_str.toInt( &ok);
               if( !ok)
               {
                  AFERRAR("Service has invalid maximum hosts count [Line %d - Col %d]: \"%s\"\n",
                     topDomElement.lineNumber(), topDomElement.columnNumber(), topDomElement.tagName().toUtf8().data());
                  return;
               }
            }
            addServiceLimit( servicename.toUtf8().data(), servicemaxcount, servicemaxhosts);
            if( Verbose) printf("\t%s: maxcount = %d maxhosts = %d\n", servicename.toUtf8().data(), servicemaxcount, servicemaxhosts);
         }
         else
         {
            printf("Unknown element [Line %d - Col %d]: \"%s\"\n",
               topDomElement.lineNumber(), topDomElement.columnNumber(), topDomElement.tagName().toUtf8().data());
         }
      }
      patDomNode = patDomNode.nextSibling();
   }
   if( count) valid = true;
   else
      AFERRAR("No patterns founded in \"%s\"\n", filename.c_str());
}

Farm::~Farm()
{
   while( ptr_first != NULL)
   {
      ptr_last = ptr_first;
      ptr_first = ptr_first->ptr_next;
      delete ptr_last;
   }
   for( std::map<std::string, ServiceLimit*>::const_iterator it = servicelimits.begin(); it != servicelimits.end(); it++)
      delete (*it).second;
}

void Farm::addServiceLimit( const std::string & name, int maxcount, int maxhosts)
{
   if( servicelimits.find( name) != servicelimits.end())
   {
      AFERRAR("Farm::addService: Service \"%s\" already exists.\n", name.c_str())
      return;
   }
   if( maxcount < -1 )
   {
      AFERRAR("Farm::addService: Service \"%s\" maxcount value is invalid \"%d\". Setting as \"-1\"\n", name.c_str(), maxcount)
      maxcount = -1;
   }
   if( maxhosts < -1 )
   {
      AFERRAR("Farm::addService: Service \"%s\" maxhosts value is invalid \"%d\". Setting as \"-1\"\n", name.c_str(), maxhosts)
      maxhosts = -1;
   }
   if(( maxcount == -1 ) && ( maxhosts == -1 ))
   {
      AFERRAR("Farm::addService: Service \"%s\" has and maxcount and maxhosts negative values.\n", name.c_str())
      return;
   }
   servicelimits[name] = new ServiceLimit( maxcount, maxhosts);
}

bool Farm::addPattern( Pattern * patern)
{
   if( patern->isValid() == false)
   {
      AFERRAR("Farm::addPattern: invalid pattern \"%s\"\n", patern->getName().c_str());
      return false;
   }
   if( ptr_first == NULL)
   {
      ptr_first = patern;
   }
   else
   {
      ptr_last->ptr_next = patern;
   }
   ptr_last = patern;
   count++;
   return true;
}

void Farm::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full) stream << std::endl;
   stream << "Farm filename = \"" << filename << "\":";
   Pattern * patern = ptr_first;
   while( patern != NULL)
   {
      stream << std::endl;
      patern->generateInfoStream( stream, full);
      patern = patern->ptr_next;
   }
   if( full ) stream << "\nServices Limits:";
   else stream << " limits:";
   for( std::map<std::string, ServiceLimit*>::const_iterator it = servicelimits.begin(); it != servicelimits.end(); it++)
   {
      stream << std::endl;
      if( full ) stream << "   ";
      stream << (*it).first << ": ";
      (*it).second->generateInfoStream( stream, full);
   }
}

void Farm::stdOut( bool full) const
{
   std::ostringstream stream;
   generateInfoStream( stream, full);
   std::cout << stream.str() << std::endl;
}

bool Farm::getHost( const std::string & hostname, Host & host, std::string & name, std::string & description) const
{
   Pattern * ptr = NULL;
   for( Pattern * p = ptr_first; p != NULL; p = p->ptr_next)
   {
      if( p->match( hostname)) ptr = p;
      if( ptr == NULL) continue;
      ptr->getHost( host);
   }
   if( ptr == NULL ) return false;
   name = ptr->getName();
   description = ptr->getDescription();
   return true;
}

bool Farm::serviceLimitCheck( const std::string & service, const std::string & hostname) const
{
   // Find a service:
   std::map< std::string, ServiceLimit * >::const_iterator it = servicelimits.find( service);

   // If there is no limits description, it can be run in anyway:
   if( it == servicelimits.end()) return true;

   return (*it).second->canRun( hostname);
}

bool Farm::serviceLimitAdd( const std::string & service, const std::string & hostname)
{
   // Find a service:
   std::map< std::string, ServiceLimit * >::const_iterator it = servicelimits.find( service);

   // If there is no limits description, we do not add it:
   if( it == servicelimits.end()) return true;

   return (*it).second->increment( hostname);
}

bool Farm::serviceLimitRelease( const std::string & service, const std::string & hostname)
{
   // Find a service:
   std::map< std::string, ServiceLimit * >::const_iterator it = servicelimits.find( service);

   // If there is no limits description, we do not add it:
   if( it == servicelimits.end()) return true;

   return (*it).second->releaseHost( hostname);
}

void Farm::servicesLimitsGet( const Farm & other)
{
   for( std::map<std::string, ServiceLimit*>::iterator it = servicelimits.begin(); it != servicelimits.end(); it++)
   {
      for( std::map<std::string, ServiceLimit*>::const_iterator oit = other.servicelimits.begin(); oit != other.servicelimits.end(); oit++)
      {
         if((*it).first == (*oit).first)
            (*it).second->getLimits(*((*oit).second));
      }
   }
}

const std::string Farm::serviceLimitsInfoString( bool full) const
{
   if( servicelimits.size() < 1 ) return std::string();

   std::ostringstream stream;

   if( full ) stream << "Services Limits:";
   else stream << " limits:";
   for( std::map<std::string, ServiceLimit*>::const_iterator it = servicelimits.begin(); it != servicelimits.end(); it++)
   {
      stream << std::endl;
      if( full ) stream << "   ";
      stream << (*it).first << ": ";
      (*it).second->generateInfoStream( stream, full);
   }

   return stream.str();
}
