#include "afcontainer.h"

#include <stdio.h>

#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/regexp.h"

#include "afcommon.h"
#include "aflist.h"
#include "afcontainerit.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfContainer::AfContainer(  std::string ContainerName, int MaximumSize):
   count( 0),
   size( MaximumSize),
   name( ContainerName),
   first_ptr( NULL),
   last_ptr( NULL),
   initialized( false)
{
   nodesTable = new af::Node*[size];
   if( nodesTable == NULL)
   {
      AFERRAR("AfContainer::AfContainer: cant't allocate memory for %d nodes.\n", size);
      return;
   }
   AFINFA("AfContainer::AfContainer: %d bytes allocated for table at %p\n", size*sizeof(af::Node*), nodesTable);
   for( int i = 0; i < size; i++ ) nodesTable[i] = NULL;

   initialized = true;
}

AfContainer::~AfContainer()
{
AFINFO("AfContainer::~AfContainer:")
   size = 0;
   while( first_ptr != NULL)
   {
      last_ptr = first_ptr;
      first_ptr = first_ptr->next_ptr;
      delete last_ptr;
   }
   if( nodesTable != NULL) delete [] nodesTable;
}

int AfContainer::add( af::Node *node)
{
   if( node == NULL )
   {
      AFERROR("AfContainer::add: node == NULL.")
      return 0;
   }
   if( count >= size-1)
   {
      AFERRAR("AfContainer::add: maximum number of nodes = %d reached.", count)
      return 0;
   }

   int newId = node->id;
   bool founded = false;

   if( newId != 0)
   {
      if( nodesTable[newId] != NULL )
      {
         AFERRAR("AfContainer::add: node->id = %d already exists.", newId)
      }
      else
      {
         founded = true;
      }
   }
   else
   {
      for( newId = 1; newId < size; newId++)
      {
         if( nodesTable[newId] == NULL )
         {
            founded = true;
            break;
         }
      }
   }

   if( !founded )
   {
      newId = 0;
   }
   else
   {
      node->id = newId;

//
// get an unique name
      {
      std::string origname = node->name;
      int number = 1;
      for(;;)
      {
         bool unique = true;
         af::Node * another = first_ptr;
         while( another != NULL)
         {
            if((!another->isZombie()) && (another->name == node->name))
            {
               node->name = origname + '-' + af::itos( number++);
               unique = false;
               break;
            }
            another = another->next_ptr;
         }
         if( unique ) break;
      }
      }

//
// find a *before node with greater or equal priority and a node *after it
      af::Node *before = first_ptr;
      af::Node *after  = NULL;
      while( before != NULL )
      {
         if( *before < *node )
         {
            after = before;
            before = before->prev_ptr;
            break;
         }
         after = before->next_ptr;
         if( after == NULL ) break;
         before = after;
      }

      if( before == NULL)
      {
         first_ptr = node;
//         printf("before == NULL\n");
      }
      else
      {
//         printf("before:"); before->stdOut();
         before->next_ptr = node;
         node->prev_ptr = before;
      }
      if( after  == NULL)
      {
         last_ptr = node;
//         printf("after == NULL\n");
      }
      else
      {
//         printf("after:"); after->stdOut();
         after->prev_ptr = node;
         node->next_ptr = after;
      }

      nodesTable[node->id] = node;
      count++;
   }

   if( !founded )
      AFERROR("AfContainer::add: nodes table full.")
   AFINFA("AfContainer::add: new id = %u, count = %u", node->id, count)
   return newId;
}

void AfContainer::refresh( AfContainer * pointer, MonitorContainer * monitoring)
{
   time_t currnetTime = time( NULL);
   for( af::Node * node = first_ptr; node != NULL; node = node->next_ptr)
   {
      if( node->isZombie() ) continue;
      node->refresh( currnetTime, pointer, monitoring);
   }
}

af::Msg* AfContainer::generateList( int type)
{
   af::MCAfNodes mcNodes;
   for( af::Node * node = first_ptr; node != NULL; node = node->next_ptr)
   {
      if( node->isZombie() ) continue;
      mcNodes.addNode( node);
   }
   return new af::Msg( type, &mcNodes);
}

af::Msg* AfContainer::generateList( int type, const af::MCGeneral & mcgeneral)
{
   af::MCAfNodes mcNodes;
   int getcount = mcgeneral.getCount();

   for( int i = 0; i < getcount; i++)
   {
      int pos = mcgeneral.getId(i);
      if( pos >= size)
      {
         AFCommon::QueueLogError("AfContainer::generateList: position >= size");
         continue;
      }
      af::Node * node = nodesTable[ pos];
      if( node == NULL   ) continue;
      if( node->isZombie()) continue;
      mcNodes.addNode( node);
   }
   if(( getcount == 0) && (false == mcgeneral.getName().empty()))
   {
      std::string errMsg;
      af::RegExp rx;
      rx.setPattern( mcgeneral.getName(), &errMsg);
      if( rx.empty())
      {
         AFCommon::QueueLogError( std::string("AfContainer::generateList: ") + errMsg);
      }
      else
      {
         bool namefounded = false;
         for( af::Node *node = first_ptr; node != NULL; node = node->next_ptr )
         {
            if( node == NULL   ) continue;
            if( node->isZombie()) continue;
            if( rx.match( node->name))
            {
               mcNodes.addNode( node);
               if( false == namefounded) namefounded = true;
            }
         }
         if( namefounded == false )
            AFCommon::QueueLog(std::string("AfContainer::generateList: No node matches \"") + mcgeneral.getName() + ("\" founded."));
      }
   }
   return new af::Msg( type, &mcNodes);
}

bool AfContainer::setZombie( int id)
{
   if( id < 1 )
   {
      AFERRAR("AfContainer::setZombie: invalid id = %d", id)
      return false;
   }
   if( id >= size )
   {
      AFERRAR("AfContainer::setZombie: Too big id = %d < %d = maximum.", id, size)
      return false;
   }
   af::Node * node = nodesTable[ id];
   if( node == NULL )
   {
      AFERRAR("AfContainer::setZombie: No node with id=%d.", id)
      return false;
   }
   if( node->isZombie())
   {
      AFERRAR("AfContainer::setZombie: Node with id=%d already a zombie.", id)
      return false;
   }
   node->setZombie();
   return true;
}

void AfContainer::freeZombies()
{
   af::Node *node = first_ptr;
   last_ptr = NULL;
   while( node != NULL)
   {
      if((node->isZombie()) && node->unLocked())
      {
         af::Node* z_node = node;
         node = z_node->next_ptr;
         if( last_ptr != NULL)
         {
            last_ptr->next_ptr = node;
            if( node != NULL ) node->prev_ptr = last_ptr;
         }
         else
         {
            first_ptr = node;
            if( node != NULL ) first_ptr->prev_ptr = NULL;
         }
         nodesTable[ z_node->id] = NULL;

         std::list<AfList*>::iterator it = z_node->lists.begin();
         std::list<AfList*>::iterator end_it = z_node->lists.end();
         while( it != end_it) (*it++)->remove( z_node);

         delete z_node;
         count--;
      }
      else
      {
         last_ptr = node;
         node = node->next_ptr;
      }
   }
}

void AfContainer::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   bool namefounded = false;
   std::string pattern = mcgeneral.getName();
   int getcount = mcgeneral.getCount();

   if( getcount < 1 )
   {
      std::string errMsg;
      af::RegExp rx;
      rx.setPattern( pattern, &errMsg);
      if( rx.empty())
      {
         AFCommon::QueueLogError( std::string("AfContainer::action: Name pattern \"") + pattern + ("\" is invalid: ") + errMsg);
         return;
      }
      for( af::Node *node = first_ptr; node != NULL; node = node->next_ptr )
      {
         if( rx.match( node->name))
         {
            action( node, mcgeneral, type, pointer, monitoring);
            if( false == namefounded) namefounded = true;
         }
      }
   }
   else
   {
      for( int i = 0; i < getcount; i++)
      {
         int pos = mcgeneral.getId( i);
         if( pos >= size)
         {
            AFERRAR("AfContainer::action: position >= size (%d>=%d)", pos, size)
            continue;
         }
         af::Node *node = nodesTable[ pos];
         if( node == NULL ) continue;
         action( node, mcgeneral, type, pointer, monitoring);
      }
   }

   if(( getcount == 0) && ( namefounded == false))
      AFCommon::QueueLog( name + ": " + af::Msg::TNAMES[type] + ": No node matches \"" + pattern + "\" founded.");
}

void AfContainer::action( af::Node * node, const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   if( node->isLocked()) return;
   if( node->action( mcgeneral, type, pointer, monitoring) == false )
   {
      AFCommon::QueueLogError( std::string("Unknown  action: ") + af::Msg::TNAMES[type]);
      return;
   }
   switch( type)
   {
      case af::Msg::TRenderSetPriority:
      case af::Msg::TJobPriority:
      case af::Msg::TUserPriority:
         sortPriority( node);
   }
}

void AfContainer::sortPriority( af::Node * node)
{
   if( count < 2 ) return;

// extract node from list by connecting pointer of previous and next nodes
   af::Node * before  = node->prev_ptr;
   af::Node * after   = node->next_ptr;
   if(    before != NULL ) before->next_ptr = node->next_ptr;
   else first_ptr = node->next_ptr;
   if(    after  != NULL )  after->prev_ptr = node->prev_ptr;
   else last_ptr  = node->prev_ptr;

   bool lessPriorityFounded = false;
// insetring node after last node with a greater or same priority
   for( before = first_ptr; before != NULL; before = before->next_ptr )
   {
      if( *before >= *node ) continue;

      after  = before;
      before = before->prev_ptr;
      node->prev_ptr = before;
      node->next_ptr = after;
      after->prev_ptr = node;
      if( before != NULL ) before->next_ptr = node;
      else first_ptr = node;

      lessPriorityFounded = true;
      break;
   }

   if( lessPriorityFounded == false )
   {
   // push node into the end of list
      last_ptr->next_ptr = node;
      node->prev_ptr = last_ptr;
      node->next_ptr = NULL;
      last_ptr = node;
   }

   std::list<AfList*>::iterator it = node->lists.begin();
   std::list<AfList*>::iterator end_it = node->lists.end();
   while( it != end_it) (*it++)->sortPriority( node);
}
