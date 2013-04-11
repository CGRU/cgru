#include "afcontainer.h"

#include <stdio.h>

#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/regexp.h"

#include "action.h"
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
   nodesTable = new AfNodeSrv*[size];
   if( nodesTable == NULL)
   {
      AFERRAR("AfContainer::AfContainer: cant't allocate memory for %d nodes.\n", size);
      return;
   }
   AFINFA("AfContainer::AfContainer: %d bytes allocated for table at %p\n", size*sizeof(AfNodeSrv*), nodesTable);
	for( int i = 0; i < size; i++ )
		nodesTable[i] = NULL;

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

int AfContainer::add( AfNodeSrv * i_node)
{
	if( i_node == NULL )
	{
		AFERROR("AfContainer::add: node == NULL.")
		return 0;
	}
	if( count >= size-1)
	{
		AFERRAR("AfContainer::add: maximum number of nodes = %d reached.", count)
		return 0;
		}

//printf("AfContainer::add: node = %p, node->m_node = %p\n", (void*)(i_node), (void*)(i_node->m_node));
//printf("AfContainer::add: name = %s\n", i_node->m_node->m_name.c_str());

	int newId = i_node->m_node->m_id;
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
      i_node->m_node->m_id = newId;

//
// get an unique name
	{
      std::string origname = i_node->m_node->m_name;
      int number = 1;
      for(;;)
      {
         bool unique = true;
         AfNodeSrv * another = first_ptr;
         while( another != NULL)
         {
            if((!another->m_node->isZombie()) && (another->m_node->m_name == i_node->m_node->m_name))
            {
               i_node->m_node->m_name = origname + '-' + af::itos( number++);
               unique = false;
               break;
            }
            another = another->m_next_ptr;
         }
         if( unique ) break;
      }
	}

//printf("############\n");
//
// find a *before node with greater or equal priority and a node *after it
      AfNodeSrv *before = first_ptr;
      AfNodeSrv *after  = NULL;
      while( before != NULL )
      {
         if( *before < *i_node )
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
         first_ptr = i_node;
//         printf("before == NULL\n");
      }
      else
      {
//         printf("before:"); before->stdOut();
         before->m_next_ptr = i_node;
         i_node->m_prev_ptr = before;
      }
      if( after  == NULL)
      {
         last_ptr = i_node;
//         printf("after == NULL\n");
      }
      else
      {
//         printf("after:"); after->stdOut();
         after->m_prev_ptr = i_node;
         i_node->m_next_ptr = after;
      }

      nodesTable[i_node->m_node->m_id] = i_node;
      count++;
   }

   if( !founded )
      AFERROR("AfContainer::add: nodes table full.")
   //AFINFA("AfContainer::add: new id = %u, count = %u", i_node->m_id, count)
   AFINFA("AfContainer::add: new id = %u, count = %u", i_node->m_node->m_id, count)
   return newId;
}

void AfContainer::refresh( AfContainer * pointer, MonitorContainer * monitoring)
{
   time_t currnetTime = time( NULL);
   for( AfNodeSrv * node = first_ptr; node != NULL; node = node->m_next_ptr)
   {
      if( node->m_node->isZombie() ) continue;
      node->v_refresh( currnetTime, pointer, monitoring);
   }
}

af::Msg * AfContainer::generateList( int i_type, const std::string & i_type_name, const std::vector<int32_t> & i_ids, const std::string & i_mask, bool i_json)
{
	af::MCAfNodes mcnodes;
	std::ostringstream str;

	if( i_json )
		str << "{\"" << i_type_name << "\":[\n";

	if( i_ids.size())
		generateListIDs( i_type, mcnodes, str, i_ids, i_json);
	else if( i_mask.size())
		generateListMask( i_type, mcnodes, str, i_mask, i_json);
	else
		generateListAll( i_type, mcnodes, str, i_json);

	af::Msg * msg = new af::Msg();

	if( i_json )
	{
		str << "\n]}";
		std::string s = str.str();
		msg->setData( s.size(), s.c_str(), af::Msg::TJSON);
	}
	else
		msg->set( i_type, &mcnodes);

	return msg;
}

void AfContainer::generateListAll( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, bool i_json)
{
	bool added = false;

	for( AfNodeSrv * node = first_ptr; node != NULL; node = node->m_next_ptr)
	{
		if( node->m_node->isZombie() ) continue;

		if( added && i_json )
			o_str << ",\n";

		if( i_json )
			node->m_node->v_jsonWrite( o_str, i_type);
		else
			o_mcnodes.addNode( node->m_node);

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
			AFCommon::QueueLogError("AfContainer::generateListIDs: position >= size");
			continue;
		}
		AfNodeSrv * node = nodesTable[ i_ids[i]];
		if( node == NULL   )
			continue;
		if( node->m_node->isZombie())
			continue;

		if( added && i_json )
			o_str << ",\n";

		if( i_json )
			node->m_node->v_jsonWrite( o_str, i_type);
		else
			o_mcnodes.addNode( node->m_node);

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
		AFCommon::QueueLogError( std::string("AfContainer::generateListMask: ") + errMsg);
	}
	else
	{
		for( AfNodeSrv *node = first_ptr; node != NULL; node = node->m_next_ptr )
		{
			if( node == NULL   ) continue;
			if( node->m_node->isZombie()) continue;
			if( rx.match( node->m_node->m_name))
			{
				if( added && i_json )
					o_str << ",\n";

				if( i_json )
					node->m_node->v_jsonWrite( o_str, i_type);
				else
					o_mcnodes.addNode( node->m_node);

				added = true;
			}
		}
	}
}

af::Msg * AfContainer::generateList( int type, const af::MCGeneral & mcgeneral)
{
	std::string type_name;
	return generateList( type, type_name, mcgeneral.getList(), mcgeneral.getName(), false);
}

af::Msg * AfContainer::generateList( int i_type)
{
	std::string type_name;
	std::vector<int32_t> ids;
	std::string mask;
	return generateList( i_type, type_name, ids, mask, false);
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
   AfNodeSrv * node = nodesTable[ id];
   if( node == NULL )
   {
      AFERRAR("AfContainer::setZombie: No node with id=%d.", id)
      return false;
   }
   if( node->m_node->isZombie())
   {
      AFERRAR("AfContainer::setZombie: Node with id=%d already a zombie.", id)
      return false;
   }
   node->v_setZombie();
   return true;
}

void AfContainer::freeZombies()
{
   AfNodeSrv *node = first_ptr;
   last_ptr = NULL;
   while( node != NULL)
   {
      if((node->m_node->isZombie()) && node->m_node->unLocked())
      {
         AfNodeSrv* z_node = node;
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
         nodesTable[ z_node->m_node->m_id] = NULL;

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

void AfContainer::action( Action & i_action)
{
	if( i_action.ids.size())
	{
		for( int i = 0; i < i_action.ids.size(); i++)
		{
			if( i_action.ids[i] >= size)
			{
				AFERRAR("AfContainer::action: position >= size (%d>=%d)", i_action.ids[i], size)
				continue;
			}
			AfNodeSrv * node = nodesTable[i_action.ids[i]];
			if( node == NULL ) continue;
			node->action( i_action);
		}
		return;
	}

	if( i_action.mask.size())
	{
		std::string errMsg;
		af::RegExp rx;
		rx.setPattern( i_action.mask, &errMsg);
		if( rx.empty())
		{
			AFCommon::QueueLogError( std::string("AfContainer::action: Name pattern \"") + i_action.mask + ("\" is invalid: ") + errMsg);
			return;
		}
		bool namefounded = false;
		for( AfNodeSrv * node = first_ptr; node != NULL; node = node->m_next_ptr )
		{
			if( rx.match( node->m_node->m_name))
			{
				node->action( i_action);
				if( false == namefounded) namefounded = true;
			}
		}
		if( false == namefounded )
			AFCommon::QueueLog( name + ": No node matches \"" + i_action.mask + "\" founded.");
	}
}

void AfContainer::sortPriority( AfNodeSrv * i_node)
{
   if( count < 2 ) return;

// extract node from list by connecting pointer of previous and next nodes
   AfNodeSrv * before  = i_node->m_prev_ptr;
   AfNodeSrv * after   = i_node->m_next_ptr;
   if(    before != NULL ) before->m_next_ptr = i_node->m_next_ptr;
   else first_ptr = i_node->m_next_ptr;
   if(    after  != NULL )  after->m_prev_ptr = i_node->m_prev_ptr;
   else last_ptr  = i_node->m_prev_ptr;

   bool lessPriorityFounded = false;
// insetring node after last node with a greater or same priority
   for( before = first_ptr; before != NULL; before = before->m_next_ptr )
   {
      if( *before >= *i_node ) continue;

      after  = before;
      before = before->m_prev_ptr;
      i_node->m_prev_ptr = before;
      i_node->m_next_ptr = after;
      after->m_prev_ptr = i_node;
      if( before != NULL ) before->m_next_ptr = i_node;
      else first_ptr = i_node;

      lessPriorityFounded = true;
      break;
   }

   if( lessPriorityFounded == false )
   {
   // push node into the end of list
      last_ptr->m_next_ptr = i_node;
      i_node->m_prev_ptr = last_ptr;
      i_node->m_next_ptr = NULL;
      last_ptr = i_node;
   }

   std::list<AfList*>::iterator it = i_node->m_lists.begin();
   std::list<AfList*>::iterator end_it = i_node->m_lists.end();
   while( it != end_it) (*it++)->sortPriority( i_node);
}
