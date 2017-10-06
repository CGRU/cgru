#include "msg.h"
#include "client.h"

#include <stdio.h>
#include <string.h>

#include "../include/afanasy.h"
#include "../include/afversion.h"

#include "msgclasses/mctest.h"
#include "environment.h"
#include "address.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

const int Msg::Version = AFVERSION;

// Version + Type + Integer = 12 bytes.
const int Msg::SizeHeader        = 4 + 4 + 4;

const int Msg::SizeBuffer        = 1 << 14;                 ///< Message reading buffer size = 16 Kilo bytes.
const int Msg::SizeBufferLimit   = Msg::SizeBuffer << 12;   ///< Message buffer maximum size = 67 Mega bytes.
const int Msg::SizeDataMax       = Msg::SizeBufferLimit - Msg::SizeHeader;

//
//########################## Message constructors: (and destructor) ###########################
Msg::Msg( int msgType, int msgInt)
{
	construct();
	set( msgType, msgInt);
}

Msg::Msg( int msgType, Af * afClass)
{
	construct();
	set( msgType, afClass);
}

Msg::Msg( const struct sockaddr_storage * ss):
	m_address( ss)
{
	construct();
	set( TNULL);
}

Msg::Msg( const char * rawData, int rawDataLen):
	m_buffer( NULL),
	// This message not for write any more data. All data will be written in this constuctor.
	// We will only read node parameters to constuct af::Af based classes.
	m_writing( false)
{
	if( rawDataLen < Msg::SizeHeader ) // Check minimum message size.
	{
		AFERRAR("Msg::Msg: rawDataLen < Msg::SizeHeader (%d<%d).", rawDataLen, Msg::SizeHeader)
		setInvalid();
		return;
	}
	if( rawDataLen > Msg::SizeBufferLimit )  // Check maximum message size.
	{
		AFERRAR("Msg::Msg: rawDataLen > Msg::SizeHeader (%d>%d).", rawDataLen, Msg::SizeBufferLimit)
		setInvalid();
		return;
	}
	if( false == allocateBuffer( rawDataLen)) return; // Allocate memory
	memcpy( m_buffer, rawData, rawDataLen); // Copy raw data to the message internal buffer.
	rw_header( false);                     // Read header information from the buffer.
}

Msg::~Msg()
{
	if( m_buffer != NULL) delete [] m_buffer;
}
//
//########################## Message methods: #################################

void Msg::construct()
{
	m_version = Msg::Version;
	m_type = Msg::TNULL;
	m_int32 = 0;

	m_header_offset = 0;

	m_writing = false;
	m_writtensize = 0;

	m_buffer = NULL;
	allocateBuffer( Msg::SizeBuffer);
}

bool Msg::allocateBuffer( int i_size, int i_copy_len, int i_copy_offset)
{
	if( m_type == Msg::TInvalid) return false;

    if( i_size < 0 || i_size > Msg::SizeBufferLimit)
	{
		AFERRAR("Msg::allocateBuffer: size > Msg::SizeBufferLimit ( %d > %d)", i_size, Msg::SizeBufferLimit)
		setInvalid();
		return false;
	}

	char * old_buffer = m_buffer;
	m_buffer_size = i_size;
	AFINFA("Msg::allocateBuffer(%s): trying %d bytes ( %d written at %p)", TNAMES[m_type], i_size, m_writtensize, old_buffer)
	m_buffer = new char[m_buffer_size];
	if( m_buffer == NULL )
	{
		AFERRAR("Msg::allocateBuffer: can't allocate %d bytes for buffer.", m_buffer_size)
		setInvalid();
		return false;
	}

	m_data         = m_buffer      + Msg::SizeHeader;
	m_data_maxsize = m_buffer_size - Msg::SizeHeader;

	if( old_buffer != NULL )
	{
//printf("Copying old buffer: offset=%d size=%d\n", i_copy_offset, i_copy_len);
		if( i_copy_len > 0) memcpy( m_data, old_buffer + i_copy_offset, i_copy_len);
		delete [] old_buffer;
	}

	return true;
}

char * Msg::writtenBuffer( int size)
{
	if( m_type == Msg::TInvalid) return NULL;
//printf("Msg::writtenBuffer: size=%d, msgwrittensize=%d, address=%p\n", size, msgwrittensize, mdata + msgwrittensize);
	if( m_writtensize+size > m_data_maxsize)
	{
		int newsize = m_buffer_size << 3;
		if( m_writtensize+size > newsize) newsize = m_writtensize+size+Msg::SizeHeader;
		if( allocateBuffer( newsize, m_writtensize) == false ) return NULL;
	}
	char * wBuffer = m_data + m_writtensize;
//printf("Msg::writtenBuffer: size=%d, msgwrittensize=%d (wBuffer=%p)\n", size, msgwrittensize, wBuffer);
	m_writtensize += size;
	return wBuffer;
}

bool Msg::checkZero( bool outerror )
{
	if(( m_type  != Msg::TNULL ) ||
		( m_int32 != 0          ) )
	{
		if( outerror )
		{
			AFERROR("Msg::checkZero(): message is non zero ( or with data).")
		}
		setInvalid();
		return false;
	}
	return true;
}

bool Msg::set( int msgType, int msgInt)
{
	if( msgType >= Msg::TDATA)
	{
		AFERROR("Msg::set: Trying to set data message with no data.")
		setInvalid();
		return false;
	}
	m_type  = msgType;
	m_int32 = msgInt;
	rw_header( true);
	return true;
}

bool Msg::setData( int i_size, const char * i_msgData, int i_type)
{
	if(checkZero( true) == false ) return false;

	m_type = i_type;

	if(( i_size    <= 0                ) ||
		( i_size    >  Msg::SizeDataMax ) ||
		( i_msgData == NULL             ) )
	{
		AFERROR("Msg::setData(): invalid arguments.")
		setInvalid();
		return false;
	}
	m_writing = true;
	w_data( i_msgData, this, i_size);

	m_int32 = i_size;

	// Default header type for JSON - not binary
	// So we should skip binary header at all
	if(( m_type == Msg::TJSON ) || ( m_type == Msg::THTTPGET ))
		m_header_offset = Msg::SizeHeader;

	rw_header( true);

	return true;
}

void Msg::setJSONBIN()
{
	if( m_type != TJSON )
	{
		AFERROR("Can't set JSON to binary, as message is not JSON:");
		v_stdOut();
		return;
	}
	m_type = TJSONBIN;
	m_header_offset = 0;
	rw_header( true);
}
/*
bool Msg::setJSON_headerBin( const std::string & i_str)
{
	bool result = setData( i_str.size(), i_str.c_str(), TJSON);

	// Reset header offset, as it was set above to Msg::SizeHeader
	m_header_offset = 0;

	return result;
}
*/
bool Msg::set( int msgType, Af * afClass)
{
	if(checkZero( true) == false ) return false;

	m_type = msgType;
	if( m_type < TDATA)
	{
		AFERROR("Msg::set: trying to set Af class with nondata type message.")
		m_type = Msg::TInvalid;
		rw_header( true);
		return false;
	}
	m_writing = true;
	afClass->write( this);
	if( m_type == Msg::TInvalid)
	{
		delete m_buffer;
		m_buffer = NULL;
		m_type = TNULL;
		allocateBuffer(100);
		w_String( "Maximum message size overload !", this);
		return false;
	}
	m_int32 = m_writtensize;
	rw_header( true);
	return true;
}

bool Msg::setString( const std::string & str)
{
	if(checkZero( true) == false ) return false;
	m_type = TString;
	m_writing = true;
	w_String( str, this);
	m_int32 = m_writtensize;
	rw_header( true);
	return true;
}

bool Msg::setInfo( const std::string & i_kind, const std::string & i_text)
{
	if(checkZero( true) == false ) return false;
	m_type = TInfo;
	m_writing = true;
	w_String( i_kind, this);
	w_String( i_text, this);
	m_int32 = m_writtensize;
	rw_header( true);
	return true;
}

bool Msg::setStringList( const std::list<std::string> & stringlist)
{
	if(checkZero( true) == false ) return false;
	m_type = TStringList;
	m_writing = true;
	w_StringList( stringlist, this);
	m_int32 = m_writtensize;
	rw_header( true);
	return true;
}

bool Msg::getString( std::string & str)
{
	if( m_type != TString)
	{
		AFERROR("Msg::getString: type is not TString.")
		return false;
	}
	rw_String( str, this);
	// Reset written size to let to get string again.
	resetWrittenSize();
	return true;
}
const std::string Msg::getString()
{
	std::string str;
	getString( str);
	return str;
}
bool Msg::getInfo( std::string & o_kind, std::string & o_text)
{
	if( m_type != TInfo)
	{
		AFERROR("Msg::getInfo: type is not TInfo.")
		return false;
	}
	rw_String( o_kind, this);
	rw_String( o_text, this);
	// Reset written size to let to get string again.
	resetWrittenSize();
	return true;
}

bool Msg::getStringList( std::list<std::string> & stringlist)
{
	if( m_type != TStringList)
	{
		AFERROR("Msg::getStringList: type is not TQStringList.")
		return false;
	}
	rw_StringList( stringlist, this);
	// Reset written size to let to get strings again.
	resetWrittenSize();
	return true;
}

void Msg::setHeader( int i_type, int i_size, int i_offset, int i_bytes)
{
//printf("Msg::setHeader:\n");
	m_version = af::Msg::Version;
	m_type    = i_type;
	m_int32   = i_size;
	
	checkValidness();

	if( m_type >= Msg::TDATA)
	{
		if( m_data_maxsize < m_int32)
		{
			allocateBuffer( m_int32+Msg::SizeHeader, i_bytes - i_offset, i_offset);
			rw_header( true );
		}
		else
		{
			if(( i_offset > 0 ) && ( i_bytes > 0 ))
				memmove( m_buffer + af::Msg::SizeHeader, m_buffer + i_offset, i_bytes - i_offset); 
		}
	}

	return;
}

void Msg::readHeader( int bytes)
{
	rw_header( false );
	if(( m_type >= Msg::TDATA) && ( m_data_maxsize < m_int32))
	{
		allocateBuffer( m_int32+Msg::SizeHeader, bytes-Msg::SizeHeader);
		rw_header( true );
	}
}

void Msg::v_readwrite( Msg * msg)
{
AFERROR("Msg::readwrite( Msg * msg): - Invalid call, use Msg::readwrite( bool write )")
}

void Msg::rw_header( bool write)
{
	if( write && ( false == checkValidness()))
	{
		AFERROR("Msg::rw_header: Message is invalid.")
	}

	static const int int32_size = 4;
	int offset = 0;
	rw_int32( m_version, m_buffer+offset, write); offset+=int32_size;
	rw_int32( m_type,    m_buffer+offset, write); offset+=int32_size;
	rw_int32( m_int32,   m_buffer+offset, write); offset+=int32_size;

	if(( false == write ) && ( false == checkValidness()))
	{
		AFERROR("Msg::rw_header: Message is invalid.")
	}
	m_writtensize = 0;
}

void Msg::setTypeHTTP()
{
	if( m_type != Msg::TJSON )
	{
		AFERROR("Msg::setTypeHTTP(): Type is not JSON.")
		v_stdOut();
		return;
	}
	m_type = Msg::THTTP;
	m_header_offset = Msg::SizeHeader;
}

bool Msg::checkValidness()
{
	if( m_type >= Msg::TLAST)
	{
		AFERRAR("Msg::checkValidness: type >= Msg::TLAST ( %d >= %d )", m_type, Msg::TLAST)
		setInvalid();
		return false;
	}
	if( m_type >= Msg::TDATA)
	{
		if( m_int32 == 0 )
		{
			AFERROR("Msg::checkValidness: data type with zero length")
			setInvalid();
			return false;
		}
		if( m_int32 > Msg::SizeDataMax )
		{
			AFERRAR("Msg::checkValidness: message with data length > Msg::SizeDataMax ( %d > %d )", m_int32, Msg::SizeDataMax)
			setInvalid();
			return false;
		}
	}
	if( m_version != Msg::Version)
	{
		if( af::Environment::isClient())
			AFERRAR("Msg::checkValidness: Version mismatch: Recieved(%d) != Library(%d)", m_version, Msg::Version)
		m_type = Msg::TVersionMismatch;
		m_int32 = 0;
		return true;
	}

	return true;
}

void Msg::setInvalid()
{
	m_type = Msg::TInvalid;
	m_int32 = 0;
	rw_header( true);
}

void Msg::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	if( m_type <= Msg::TLAST)
		stream << Msg::TNAMES[m_type];
	else
		stream << "!UNKNOWN!(" << m_type << ")";
	stream << "[" << writeSize() << "]";
}

void Msg::stdOutData( bool withHeader)
{
	if( withHeader) v_stdOut( true);

	switch( m_type)
	{
	case Msg::TDATA:
	case Msg::THTTP:
	case Msg::TJSON:
	case Msg::TJSONBIN:
	{
		if( m_data[0] == '/')
			break;
		static int unused;
		unused = ::write( 1, m_data, m_int32);
		unused = ::write( 1, "\n", 1);
		break;
	}
	case Msg::TTESTDATA:
	{
		MCTest( this).v_stdOut( true);
		break;
	}
	case Msg::TString:
	{
		std::string str;
		getString( str);
		std::cout << str;
		std::cout << std::endl;
		break;
	}
	case Msg::TStringList:
	{
		std::list<std::string> strlist;
		rw_StringList( strlist, this);
		for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++)
			std::cout << *it << std::endl;
		break;
	}
	}
}

const char * Msg::TNAMES[]=
{
	/*------------ NONDATA MESSAGES ----------------------*/
	/// Default message with default type - zero. Only this type can be changed by \c set function.
	"TNULL",

	/// Message set to this type itself, when reading.
	"TVersionMismatch",

	/// Invalid message. This message type generated by constructors if wrong arguments provieded.
	"TInvalid",

	"TConfirm",                   ///< Simple answer with no data to confirm something.

	/// Request messages, sizes, quantities statistics.
	"TStatRequest",

	/*- Monitor messages -*/
	"TMonitorId",                 ///< Id for new Monitor. Server sends it back when new Talk registered.
	"TMonitorUpdateId",           ///< Update Monitor with given id ( No information for updating Monitor needed).
	"TMonitorDeregister",         ///< Deregister monitor with given id.

	/*- Render messages -*/
	/** When Server successfully registered new Render it's send back it's id.**/
	"TRenderId",
	"TRenderDeregister",          ///< Deregister Render with given id.

	/*- Job messages -*/
	"TJobsWeightRequest",         ///< Request all jobs weight.


	"TRESERVED00",
	"TRESERVED01",
	"TRESERVED02",
	"TRESERVED03",
	"TRESERVED04",
	"TRESERVED05",
	"TRESERVED06",
	"TRESERVED07",
	"TRESERVED08",
	"TRESERVED09",

	/*---------------------------------------------------------------------------------------------------------*/
	/*--------------------------------- DATA MESSAGES ---------------------------------------------------------*/
	/*---------------------------------------------------------------------------------------------------------*/


	"TDATA",                      ///< Some data.
	"TTESTDATA",                  ///< Test some data transfer.
	"TJSON",                      ///< JSON with text header
	"TJSONBIN",                   ///< JSON with binary header
	"THTTP",                      ///< HTTP - with JSON POST data
	"THTTPGET",                   ///< HTTP Get request.
	"TString",                    ///< QString text message.
	"TStringList",                ///< QStringList text message.
	"TInfo",                      ///< Some info string for GUI to show.

	"TStatData",                  ///< Statistics data.

	/*- Monitor messages -*/
	"TMonitor",                   ///< Server sends it for a new registered monitor.
	"TMonitorsList",              ///< Message with a list of online Monitors.
	"TMonitorEvents",             ///< Events.

	/*- Render messages -*/
	/** Sent by Render on start, when it's server begin to listen post.
	And when Render can't connect to Afanasy. Afanasy register new Render and send back it's id \c TRenderId. **/
	"TRenderRegister",
	"TRenderUpdate",              ///< Update Render, message contains its resources.
	"TRenderEvents",              ///< Server answer on render update, it contains what to do.
	"TRendersResourcesRequestIds",///< Request a list of resources of Renders with given ids.
	"TRendersList",               ///< Message with a list of Renders.
	"TRendersResources",          ///< Message with a list of resources of Renders.

	/*- Users messages -*/
	"TUsersList",                 ///< Active users information.
	"TUserJobsOrder",             ///< Jobs ids in server list order.

	/*- Job messages -*/
	"TJobsList",                  ///< Jobs list information.
	"TJobProgress",               ///< Jobs progress.
	"TJobsWeight",                ///< All jobs weight data.
	"TJob",                       ///< Job (job full data, with blocks and tasks full data).

	"TBlocksProgress",            ///< Blocks running progress data.
	"TBlocksProperties",          ///< Blocks progress and properties data.
	"TBlocks",                    ///< Blocks data.

	"TTask",                      ///< A task of some job.
	"TTaskFiles",                 ///< Task (or entire job) files

	"TRESERVED10",
	"TRESERVED11",
	"TRESERVED12",
	"TRESERVED13",
	"TRESERVED14",
	"TRESERVED15",
	"TRESERVED16",
	"TRESERVED17",
	"TRESERVED18",
	"TRESERVED19",

	"TLAST"                       ///< The last type number.
};
