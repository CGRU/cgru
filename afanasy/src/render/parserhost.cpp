#include "parserhost.h"

#include "../libafanasy/service.h"

#ifdef WINNT
//#define strcpy strcpy_s
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ParserHost::ms_DataSizeMax   = 1 << 20; // 1 Mega Byte
const int ParserHost::ms_DataSizeHalf  = ParserHost::ms_DataSizeMax  >> 1; // A half of the maximum
const int ParserHost::ms_DataShiftMin = ParserHost::ms_DataSizeHalf >> 1; // A quater of the maximum

const char* ParserHost::ms_overload_string = "\n\
\n\
\n\
\n\
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\
\n\
\n\
\n\
Maximum size reached. Output middle trunctated.\n\
\n\
\n\
\n\
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\
\n\
\n\
\n\
";

ParserHost::ParserHost( af::Service * i_service):
	m_service( i_service),
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
	m_overload_string_length = int(strlen(ms_overload_string));

	if( m_data == NULL )
	{
		AFERROR("ParserHost::ParserHost(): Can`t allocate memory for data.")
		return;
	}
}

ParserHost::~ParserHost()
{
	if( m_data != NULL) delete [] m_data;
}

void ParserHost::read( const std::string & i_mode, std::string & output)
{
	parse( i_mode, output);

	// writing output in buffer:
	//
	const char * copy_data = output.data();
	int          copy_size = output.size();
//printf("\nParserHost::read: size = %d ( datasize = %d )\n", copy_size, m_datasize);
#ifdef AFOUTPUT
printf("\"");for(int c=0;c<out_size;c++)if(copy_size[c]>=32)printf("%c", copy_size[c]);printf("\":\n");
#endif

	// Output can reach its limit.
	// In this case we shift it to cut the middle,
	// considering that all usueful information is in the begging and the end.
	if( m_datasize + copy_size > ms_DataSizeMax )
	{
//printf("m_datasize + copy_size > ms_DataSizeMax : %d + %d > %d\n", m_datasize, copy_size, ms_DataSizeMax);

		if( m_datasize < ms_DataSizeHalf )
		{
			// Current data size is less than a half,
			// We need to copy portion to reach the half:
			memcpy( m_data+m_datasize, copy_data, ms_DataSizeHalf - m_datasize);
			copy_data = copy_data + ms_DataSizeHalf - m_datasize;
			copy_size = copy_size - ( ms_DataSizeHalf - m_datasize );
			m_datasize = ms_DataSizeHalf;
		}

		// If new portion size is a half or more,
		// no existing data shifting is needed
		if( copy_size >= ms_DataSizeHalf )
		{
			// We copy new data just a half from the end:
			copy_data = copy_data + copy_size - ms_DataSizeHalf;
			copy_size = ms_DataSizeHalf;
			m_datasize  = ms_DataSizeHalf;
//printf("copy_size >= ms_DataSizeHalf ( %d >= %d )\n", copy_size, ms_DataSizeHalf );
		}
		else
		{
			int shift = m_datasize + copy_size - ms_DataSizeMax;

			// If we can shift by a quater, we do it:
			if(( shift < ms_DataShiftMin ) && ( m_datasize > ms_DataSizeHalf + ms_DataShiftMin ))
			{
				shift = ms_DataShiftMin;
				// - this needed to prevent data shifting for each new incoming byte
			}

			// Shifting existing data:
			int move_size = m_datasize - ms_DataSizeHalf - shift;
			// This should be always > 0, as copy_size < ms_DataSizeHalf
			// explanation:
			//int move_size = m_datasize - ms_DataSizeHalf - m_datasize - copy_size + ms_DataSizeMax;
			//int move_size = - ms_DataSizeHalf - copy_size + ms_DataSizeMax;
			//int move_size = ms_DataSizeHalf - copy_size;

//printf("shift = %d, size = %d\n", shift, move_size);
			if( move_size > 0 )
			{
				// - just check if this algorithm has a bag
				memmove( m_data+ms_DataSizeHalf, m_data+ms_DataSizeHalf+shift, move_size);
				m_datasize -= shift;
			}
			else
				AFERRAR("ParserHost::read: move_size = %d < 0", move_size)
		}

		// Copy overload sting just before the middle of the data, if it was not yet:
		if( m_overload == false )
		{
//printf("Copying overload string.\n");
			strncpy( m_data+ms_DataSizeHalf-m_overload_string_length, ms_overload_string, m_overload_string_length);
			m_overload = true;
		}

	}

//printf("memcpy: datasize=%d, copysize=%d\n", m_datasize, copy_size);

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

void ParserHost::parse( const std::string & i_mode, std::string & output)
{
	bool _warning         = false;
	bool _error           = false;
	bool _badresult       = false;
	bool _finishedsuccess = false;

	m_service->parse( i_mode, output, m_percent, m_frame, m_percentframe,
		m_activity, m_report,
		_warning, _error, _badresult, _finishedsuccess);

	if ( _error           ) m_error           = true;
	if ( _warning         ) m_warning         = true;
	if ( _badresult       ) m_badresult       = true;
	if ( _finishedsuccess ) m_finishedsuccess = true;
#ifdef AFOUTPUT
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
