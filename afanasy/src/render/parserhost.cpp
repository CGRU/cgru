#include "parserhost.h"

#ifdef WINNT
//#define strcpy strcpy_s
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ParserHost::ms_DataSizeMax   = 1 << 20;
const int ParserHost::ms_DataSizeHalf  = ParserHost::ms_DataSizeMax  >> 1;
const int ParserHost::ms_DataSizeShift = ParserHost::ms_DataSizeHalf >> 1;

const char* ParserHost::ms_overload_string =
"\n\n\n\
	###########################################################   \n\
	###   Maximum size reached. Output middle trunctated.   ###   \n\
	###########################################################   \n\
\n\n\n";

ParserHost::ParserHost( const std::string & task_type, int num_frames):
	m_parser( NULL),
	m_type( task_type),
	m_numframes( num_frames),
	m_percent( 0),
	m_frame( 0),
	m_percentframe( 0),
	m_error( false),
	m_warning( false),
	m_badresult( false),
	m_finishedsuccess( false),
	m_data( NULL),
	m_datasize( 0),
	m_overload( false)
{
	m_data = new char[ms_DataSizeMax];
	m_overload_string_length = int(strlen(ms_overload_string)+1);

	if( m_data == NULL )
	{
		printf( "ParserHost::ParserHost(): Can`t allocate memory for data.");
		return;
	}

	if( false == m_type.empty())
	{
		m_parser = new af::Parser( m_type, m_numframes);
		if( m_parser->isInitialized() == false)
		{
			delete m_parser;
			m_parser = NULL;
		}
	}
}

ParserHost::~ParserHost()
{
	if( m_parser != NULL) delete m_parser;
	if( m_data   != NULL) delete [] m_data;
}

void ParserHost::read( std::string & output)
{
	parse( output);
	const char * out_data = output.data();
	int          out_size = output.size();

#ifdef AFOUTPUT
printf("\"");for(int c=0;c<out_size;c++)if(out_data[c]>=32)printf("%c", out_data[c]);printf("\":\n");
#endif

	// writing output in buffer
	//
//printf("\nParser::read: size = %d ( datasize = %d )\n", size, datasize);
	const char * copy_data = out_data;
	int          copy_size = out_size;
	if( (m_datasize+output.size()) > ms_DataSizeMax )
	{
//printf("(datasize+size) > DataSizeMax : (%d+%d)>%d\n", datasize, size, DataSizeMax);
		if( m_datasize < ms_DataSizeHalf )
		{
			memcpy( m_data+m_datasize, out_data, ms_DataSizeHalf-m_datasize);
			copy_data = out_data + ms_DataSizeHalf - m_datasize ;
			copy_size = out_size - ( ms_DataSizeHalf - m_datasize);
			m_datasize = ms_DataSizeHalf;
		}

		int sizeShift = ms_DataSizeShift;
		if( m_datasize+copy_size-sizeShift > ms_DataSizeMax ) sizeShift = m_datasize + copy_size - ms_DataSizeMax;
//printf("sizeShift=%d\n", sizeShift);
		if( sizeShift < m_datasize-ms_DataSizeHalf ) shiftData( sizeShift);
		else
		{
			copy_data = out_data + copy_size - ms_DataSizeHalf;
			copy_size = ms_DataSizeHalf;
			m_datasize  = ms_DataSizeHalf;
//printf("sizeShift >= datasize-DataSizeHalf ( %d >= %d-%d )\n", sizeShift, datasize, DataSizeHalf);
		}
		if( m_overload == false ) setOverload();
	}

//printf("memcpy: datasize=%d, copysize=%d, size=%d\n", datasize, copy_size, size);

	memcpy( m_data+m_datasize, copy_data, copy_size);
	m_datasize += copy_size;

/*#ifdef AFOUTPUT
fflush( stdout);
printf("\n##############################   ParserHost::read: ##############################\n");
fflush( stdout);
::write( 1, data, datasize);
fflush( stdout);
printf("\n#############################################################################\n", datasize);
fflush( stdout);
#endif*/

//printf("end: datasize = %d\n", datasize);
}

bool ParserHost::shiftData( int shift)
{
	if( shift < 0 )
	{
		AFERRAR("ParserHost::shiftData: shift < 0 (%d<0)\n", shift);
		return false;
	}
	if( shift == 0 ) return true;
	memcpy( m_data+ms_DataSizeHalf, m_data+ms_DataSizeHalf+shift, m_datasize-ms_DataSizeHalf-shift);
	m_datasize -= shift;
	return true;
}

void ParserHost::setOverload()
{
	strcpy( m_data+ms_DataSizeHalf-m_overload_string_length, ms_overload_string);
	m_overload = true;
}

void ParserHost::parse( std::string & output)
{
	if( m_parser )
	{
		bool _warning         = false;
		bool _error           = false;
		bool _badresult       = false;
		bool _finishedsuccess = false;

		m_parser->parse( output, m_percent, m_frame, m_percentframe, m_activity, _warning, _error, _badresult, _finishedsuccess);

		if ( _error           ) m_error           = true;
		if ( _warning         ) m_warning         = true;
		if ( _badresult       ) m_badresult       = true;
		if ( _finishedsuccess ) m_finishedsuccess = true;
#ifdef AFOUTPUds
		printf("PERCENT: %d%%", m_percent);
		printf("; FRAME: %d", m_frame);
		printf("; PERCENTFRAME: %d%%", m_percentframe);
		if( _error           ) printf("; ERROR");
		if( _warning         ) printf("; WARNING");
		if( _badresult       ) printf("; BAD RESULT");
		if( _finishedsuccess ) printf("; FINISHED SUCCESS");
		printf("\n");
#endif
	}
}
