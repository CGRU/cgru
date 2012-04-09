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
      first_ptr = first_ptr->m_next_ptr;
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

   int newId = node->m_id;
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
      node->m_id = newId;

//
// get an unique name
      {
      std::string origname = node->m_name;
      int number = 1;
      for(;;)
      {
         bool unique = true;
         af::Node * another = first_ptr;
         while( another != NULL)
         {
            if((!another->isZombie()) && (another->m_name == node->m_name))
            {
               node->m_name = origname + '-' + af::itos( number++);
               unique = false;
               break;
            }
            another = another->m_next_ptr;
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
            before = before->m_prev_ptr;
            break;
         }
         after = before->m_next_ptr;
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
         before->m_next_ptr = node;
         node->m_prev_ptr = before;
      }
      if( after  == NULL)
      {
         last_ptr = node;
//         printf("after == NULL\n");
      }
      else
      {
//         printf("after:"); after->stdOut();
         after->m_prev_ptr = node;
         node->m_next_ptr = after;
      }

      nodesTable[node->m_id] = node;
      count++;
   }

   if( !founded )
      AFERROR("AfContainer::add: nodes table full.")
   AFINFA("AfContainer::add: new id = %u, count = %u", node->m_id, count)
   return newId;
}

void AfContainer::refresh( AfContainer * pointer, MonitorContainer * monitoring)
{
   time_t currnetTime = time( NULL);
   for( af::Node * node = first_ptr; node != NULL; node = node->m_next_ptr)
   {
      if( node->isZombie() ) continue;
      node->refresh( currnetTime, pointer, monitoring);
   }
}

af::Msg * AfContainer::generateList( int i_type, const std::vector<int32_t> & i_ids, const std::string & i_mask, bool i_json)
{
	af::MCAfNodes mcnodes;
	std::ostringstream str;

	if( i_json )
		str << "{\n";

	if( i_ids.size())
		generateListIDs( i_type, mcnodes, str, i_ids, i_json);
	else if( i_mask.size())
		generateListMask( i_type, mcnodes, str, i_mask, i_json);
	else
		generateListAll( i_type, mcnodes, str, i_json);

	af::Msg * msg = new af::Msg();

	if( i_json )
	{
		str << "\n}";
		std::string s = str.str();
		msg->setData( s.size()+1, s.c_str(), af::Msg::TJSON);
	}
	else
		msg->set( i_type, &mcnodes);

	return msg;
}

void AfContainer::generateListAll( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, bool i_json)
{
	bool added = false;

	for( af::Node * node = first_ptr; node != NULL; node = node->m_next_ptr)
	{
		if( node->isZombie() ) continue;

		if( added && i_json )
			o_str << ",\n";

		if( i_json )
			node->v_jsonWrite( o_str, i_type);
		else
			o_mcnodes.addNode( node);

		added = true;
	}
}

void AfContainer::generateListIDs( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::vector<int32_t> & i_ids, bool i_json)
{
	bool added = false;

	for( int i = 0; i < i_ids.size(); i++)
	{
		if( i_ids[i] >= size)
		{
			AFCommon::QueueLogError("AfContainer::generateList: position >= size");
			continue;
		}
		af::Node * node = nodesTable[ i_ids[i]];
		if( node == NULL   )
			continue;
		if( node->isZombie())
			continue;

		if( added && i_json )
			o_str << ",\n";

		if( i_json )
			node->v_jsonWrite( o_str, i_type);
		else
			o_mcnodes.addNode( node);

		added = true;
	}
}

void AfContainer::generateListMask( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::string & i_mask, bool i_json)
{
	if( false == i_mask.size()) return;

	bool added = false;

	std::string errMsg;
	af::RegExp rx;
	rx.setPattern( i_mask, &errMsg);
	if( rx.empty())
	{
		AFCommon::QueueLogError( std::string("AfContainer::generateList: ") + errMsg);
	}
	else
	{
		bool namefounded = false;
		for( af::Node *node = first_ptr; node != NULL; node = node->m_next_ptr )
		{
			if( node == NULL   ) continue;
			if( node->isZombie()) continue;
			if( rx.match( node->m_name))
			{
				if( added && i_json )
					o_str << ",\n";

				if( i_json )
					node->v_jsonWrite( o_str, i_type);
				else
					o_mcnodes.addNode( node);

				added = true;

				if( false == namefounded)
					namefounded = true;
			}
		}
		if( namefounded == false )
		AFCommon::QueueLog(std::string("AfContainer::generateList: No node matches \"") + i_mask + ("\" founded."));
	}
}

af::Msg * AfContainer::generateList( int type, const af::MCGeneral & mcgeneral)
{
	return generateList( type, mcgeneral.getList(), mcgeneral.getName(), false);
}

af::Msg * AfContainer::generateList( int i_type)
{
	std::vector<int32_t> ids;
	std::string mask;
	return generateList( i_type, ids, mask, false);
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
         node = z_node->m_next_ptr;
         if( last_ptr != NULL)
         {
            last_ptr->m_next_ptr = node;
            if( node != NULL ) node->m_prev_ptr = last_ptr;
         }
         else
         {
            first_ptr = node;
            if( node != NULL ) first_ptr->m_prev_ptr = NULL;
         }
         nodesTable[ z_node->m_id] = NULL;

         std::list<AfList*>::iterator it = z_node->m_lists.begin();
         std::list<AfList*>::iterator end_it = z_node->m_lists.end();
         while( it != end_it) (*it++)->remove( z_node);

         delete z_node;
         count--;
      }
      else
      {
         last_ptr = node;
         node = node->m_next_ptr;
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
      for( af::Node *node = first_ptr; node != NULL; node = node->m_next_ptr )
      {
         if( rx.match( node->m_name))
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
   af::Node * before  = node->m_prev_ptr;
   af::Node * after   = node->m_next_ptr;
   if(    before != NULL ) before->m_next_ptr = node->m_next_ptr;
   else first_ptr = node->m_next_ptr;
   if(    after  != NULL )  after->m_prev_ptr = node->m_prev_ptr;
   else last_ptr  = node->m_prev_ptr;

   bool lessPriorityFounded = false;
// insetring node after last node with a greater or same priority
   for( before = first_ptr; before != NULL; before = before->m_next_ptr )
   {
      if( *before >= *node ) continue;

      after  = before;
      before = before->m_prev_ptr;
      node->m_prev_ptr = before;
      node->m_next_ptr = after;
      after->m_prev_ptr = node;
      if( before != NULL ) before->m_next_ptr = node;
      else first_ptr = node;

      lessPriorityFounded = true;
      break;
   }

   if( lessPriorityFounded == false )
   {
   // push node into the end of list
      last_ptr->m_next_ptr = node;
      node->m_prev_ptr = last_ptr;
      node->m_next_ptr = NULL;
      last_ptr = node;
   }

   std::list<AfList*>::iterator it = node->m_lists.begin();
   std::list<AfList*>::iterator end_it = node->m_lists.end();
   while( it != end_it) (*it++)->sortPriority( node);
}
