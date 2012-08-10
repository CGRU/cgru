#include "action.h"

#include "../libafanasy/environment.h"

#include "afcommon.h"

Action::Action( af::Msg * i_msg, ThreadArgs * i_args):
	jobs( i_args->jobs),
	monitors( i_args->monitors),
	renders( i_args->renders),
	talks( i_args->talks),
	users( i_args->users),
	permissions( 0),
	m_buffer( NULL),
	m_valid( false)
{
	std::string error;
	m_buffer = af::jsonParseMsg( m_document, i_msg, &error);
	if( m_buffer == NULL )
	{
		AFCommon::QueueLogError( error);
		return;
	}

	data = &m_document["action"];
	if( false == data->IsObject())
	{
		AFCommon::QueueLogError("JSON action is not an object.");
		return;
	}

	af::jr_int32vec("ids", ids, *data);
	if( ids.size() == 0 )
	{
		af::jr_string("mask", mask, *data);
		if( mask.empty())
		{
			AFCommon::QueueLogError("JSON action should have nodes ids or mask to operate with.");
			return;
		}
	}

	af::jr_string("type", type, *data);
	if( type.empty())
	{
		AFCommon::QueueLogError("JSON action type is not set.");
		return;
	}

	af::jr_string("user_name", user_name, *data);
	af::jr_string("host_name", host_name, *data);
	if( user_name.empty())
	{
		AFCommon::QueueLogError("Action should have a not empty \"user_name\" string.");
		return;
	}
	if( host_name.empty())
	{
		AFCommon::QueueLogError("Action should have a not empty \"host_name\" string.");
		return;
	}
	author = user_name + '@' + host_name;

	if( i_msg->isMagicInvalid())
	{
		switch( af::Environment::getMagicMode())
		{
		case af::MMM_GetOnly:
			permissions |= PReadOnly;
		case af::MMM_NoTasks:
			permissions |= PNoTasks;
		}
	}

	m_valid = true;
}

Action::~Action()
{
	if( m_buffer ) delete [] m_buffer;
}
