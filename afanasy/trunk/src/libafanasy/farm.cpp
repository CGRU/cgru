#include "farm.h"

#include <string.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

const char XMLNAME_PATTERN[]          = "pattern";
const char XMLNAME_PATTERNNAME[]      = "name";
const char XMLNAME_DESCRIPTION[]      = "description";
const char XMLNAME_MASK[]             = "mask";
const char XMLNAME_OS[]               = "os";
const char XMLNAME_PROPERTIES[]       = "properties";
const char XMLNAME_POWER[]            = "power";
const char XMLNAME_RESOURCES[]        = "resources";
const char XMLNAME_DATA[]             = "data";
const char XMLNAME_CAPACITY[]         = "capacity";
const char XMLNAME_MAXTASKS[]         = "maxtasks";
const char XMLNAME_SERVISE[]          = "service";
const char XMLNAME_SERVISEREMOVE[]    = "remservice";
const char XMLNAME_SERVISENAME[]      = "name";
const char XMLNAME_SERVISECOUNT[]     = "count";
const char XMLNAME_SERVISEMAXHOSTS[]  = "maxhosts";
const char XMLNAME_SERVISEMAXCOUNT[]  = "maxcount";

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
   if( false == pathFileExists( filename))
   {
      printf("Farm file \"%s\" does not esxist.\n", filename.c_str());
      return;
   }

   int filesize = -1;
   char * buffer = fileRead( filename, filesize);
   if( buffer == NULL )
   {
      printf("Farm file \"%s\" reading error.\n", filename.c_str());
      return;
   }
   else
   {
      rapidxml::xml_document<> xmldoc;

      bool parse_error = false;

      try
      {
         xmldoc.parse<0>( buffer);
      }
      catch ( rapidxml::parse_error err)
      {
         AFERRAR("Parsing error: %s.\n", err.what())
         parse_error = true;
      }
      catch ( ... )
      {
         AFERROR("Unknown exeption.\n")
         parse_error = true;
      }

      if( false == parse_error )
      {
         rapidxml::xml_node<> * root_node = xmldoc.first_node("farm");
         if( root_node == NULL )
         {
            AFERRAR("Can't find document root \"farm\": node:\n%s\n", filename.c_str())
         }
         else
         {
            valid = getPatterns( root_node);
         }
      }
      delete [] buffer;
   }
}

bool Farm::getPatterns( const rapidxml::xml_node<> * pnode)
{

   for( rapidxml::xml_node<> * node = pnode->first_node(); node != NULL; node = node->next_sibling())
   {
      if( strcmp( XMLNAME_PATTERN, node->name()) ==  0)
      {
         std::string patname, description, mask;
         Host host;
         std::list<std::string> remservices;

         rapidxml::xml_attribute<> * attr;

         attr = node->first_attribute( XMLNAME_PATTERNNAME);
         if( attr != NULL ) patname = attr->value();

         if( patname.empty())
         {
            AFERROR("Pattern has no name.")
            return false;
         }

         for( rapidxml::xml_node<> * cnode = node->first_node(); cnode != NULL; cnode = cnode->next_sibling())
         {
            std::string cnode_name;
            if( cnode->name() != NULL ) cnode_name = cnode->name();
            if( cnode_name.empty())
            {
               AFERRAR("Empty node name in pattern \"%s\".", patname.c_str())
               return false;
            }

            if( cnode_name == XMLNAME_SERVISE )
            {
               std::string servicename, servicecount_str;
               int servicecount = 0;

               attr = cnode->first_attribute( XMLNAME_SERVISENAME);
               if( attr != NULL ) servicename = attr->value();
               if( servicename.empty())
               {
                  AFERRAR("Service has no name in pattern \"%s\".", patname.c_str())
                  return false;
               }

               attr = cnode->first_attribute( XMLNAME_SERVISECOUNT);
               if( attr != NULL ) servicecount_str = attr->value();
               if( false == servicecount_str.empty())
               {
                  bool ok;
                  servicecount = stoi( servicecount_str, &ok);
                  if( false == ok )
                  {
                     AFERRAR("Service \"%s\" has invalid count \"%s\" in pattern \"%s\".", servicename.c_str(), servicecount_str.c_str(), patname.c_str())
                     return false;
                  }
               }

               host.setService( servicename, servicecount);
            }
            else if( cnode_name == XMLNAME_SERVISEREMOVE )
            {
               std::string servicename;
               attr = cnode->first_attribute( XMLNAME_SERVISENAME);
               if( attr != NULL ) servicename = attr->value();
               if( servicename.empty())
               {
                  AFERRAR("Service to remove has no name in pattern \"%s\".", patname.c_str())
                  return false;
               }
               remservices.push_back( servicename);
            }
            else
            {
               std::string cnode_value;
               if( cnode->value() != NULL ) cnode_value = cnode->value();
               if( cnode_value.empty())
               {
                  AFERRAR("Empty node \"%s\" value in pattern \"%s\".", cnode_name.c_str(), patname.c_str())
                  return false;
               }

               bool numberOk = false;
               int number = stoi( cnode_value, &numberOk);

               if     ( cnode_name == XMLNAME_MASK          ) mask            = cnode_value;
               else if( cnode_name == XMLNAME_DESCRIPTION   ) description     = cnode_value;
               else if( cnode_name == XMLNAME_OS            ) host.os         = cnode_value;
               else if( cnode_name == XMLNAME_PROPERTIES    ) host.properties = cnode_value;
               else if( cnode_name == XMLNAME_RESOURCES     ) host.resources  = cnode_value;
               else if( cnode_name == XMLNAME_DATA          ) host.data       = cnode_value;
               else if( cnode_name == XMLNAME_CAPACITY      )
               {
                  if( false == numberOk )
                  {
                     AFERRAR("Invalid machine capacity \"%s\" in pattern \"%s\".", cnode_value.c_str(), patname.c_str())
                     return false;
                  }
                  host.capacity = number;
               }
               else if( cnode_name == XMLNAME_MAXTASKS)
               {
                  if( false == numberOk )
                  {
                     AFERRAR("Invalid machine maxtasks \"%s\" in pattern \"%s\".", cnode_value.c_str(), patname.c_str())
                     return false;
                  }
                  host.maxtasks = number;
               }
               else if( cnode_name == XMLNAME_POWER)
               {
                  if( false == numberOk )
                  {
                     AFERRAR("Invalid machine power \"%s\" in pattern \"%s\".", cnode_value.c_str(), patname.c_str())
                     return false;
                  }
                  host.power = number;
               }
            }
         }

         Pattern * pat = new Pattern( patname);
         pat->setMask( mask);
         pat->setDescription( description);
         pat->setHost( host);
         pat->remServices( remservices);
         if( addPattern( pat) == false)
         {
            delete pat;
            return false;
         }

      }
      if( strcmp( XMLNAME_SERVISE, node->name()) ==  0)
      {
         rapidxml::xml_attribute<> * attr;

         std::string servicename;
         attr = node->first_attribute( XMLNAME_SERVISENAME);
         if( attr != NULL ) servicename = attr->value();
         if( servicename.empty() == 1)
         {
            AFERROR("Service limit has no name.")
            return false;
         }

         std::string servicemaxcount_str, servicemaxhosts_str;
         int servicemaxcount = -1;
         int servicemaxhosts = -1;
         bool ok = false;

         attr = node->first_attribute( XMLNAME_SERVISEMAXCOUNT);
         if( attr != NULL ) servicemaxcount_str = attr->value();
         if( false == servicemaxcount_str.empty())
         {
            servicemaxcount = stoi( servicemaxcount_str, &ok);
            if( !ok)
            {
               AFERRAR("Service limit \"%s\" has invalid maximum total count \"%s\".", servicename.c_str(), servicemaxcount_str.c_str())
               return false;
            }
         }

         attr = node->first_attribute( XMLNAME_SERVISEMAXHOSTS);
         if( attr != NULL ) servicemaxhosts_str = attr->value();
         if( false == servicemaxhosts_str.empty())
         {
            servicemaxhosts = stoi( servicemaxhosts_str, &ok);
            if( !ok)
            {
               AFERRAR("Service limit \"%s\" has invalid maximum hosts count \"%s\".", servicename.c_str(), servicemaxhosts_str.c_str())
               return false;
            }
         }

         if(( servicemaxcount < 0 ) && ( servicemaxhosts < 0 ))
         {
            AFERRAR("Service \"%s\" has invalid limits.", servicename.c_str())
            return false;
         }

         addServiceLimit( servicename, servicemaxcount, servicemaxhosts);
      }

   }

   if( count < 1 )
   {
      AFERRAR("No patterns founded in \"%s\"", filename.c_str())
      return false;
   }

   return true;
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
      AFERRAR("Farm::addService: Service \"%s\" already exists.", name.c_str())
      return;
   }
   if( maxcount < -1 )
   {
      AFERRAR("Farm::addService: Service \"%s\" maxcount value is invalid \"%d\". Setting as \"-1\"", name.c_str(), maxcount)
      maxcount = -1;
   }
   if( maxhosts < -1 )
   {
      AFERRAR("Farm::addService: Service \"%s\" maxhosts value is invalid \"%d\". Setting as \"-1\"", name.c_str(), maxhosts)
      maxhosts = -1;
   }
   if(( maxcount == -1 ) && ( maxhosts == -1 ))
   {
      AFERRAR("Farm::addService: Service \"%s\" has and maxcount and maxhosts negative values.", name.c_str())
      return;
   }
   servicelimits[name] = new ServiceLimit( maxcount, maxhosts);
}

bool Farm::addPattern( Pattern * patern)
{
   if( patern->isValid() == false)
   {
      AFERRAR("Farm::addPattern: invalid pattern \"%s\"", patern->getName().c_str())
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
