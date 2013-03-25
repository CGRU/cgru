#pragma once

#include "../include/afjob.h"

#include "../libafanasy/parser.h"

class ParserHost
{

public:

	ParserHost( const std::string & task_type, int num_frames);
	~ParserHost();

	void read( std::string & output);

	inline int getPercent()           const { return m_percent;         }
	inline int getFrame()             const { return m_frame;           }
	inline int getPercentFrame()      const { return m_percentframe;    }
	inline std::string getActivity()  const { return m_activity;        }
	inline int hasWarning()           const { return m_warning;         }
	inline int hasError()             const { return m_error;           }
	inline int isBadResult()          const { return m_badresult;       }
	inline int isFinishedSuccess()    const { return m_finishedsuccess; }
	inline char* getData( int *size ) const { *size = m_datasize; return m_data;}

private:
	af::Parser * m_parser;

	std::string m_type;
	int m_numframes;

	int  m_percent;
	int  m_frame;
	int  m_percentframe;
	bool m_error;
	bool m_warning;
	bool m_badresult;
	bool m_finishedsuccess;
	std::string m_activity;

	char*              m_data;
	int                m_datasize;
	static const int   ms_DataSizeMax;
	static const int   ms_DataSizeHalf;
	static const int   ms_DataSizeShift;
	bool               m_overload;
	static const char* ms_overload_string;
	int                m_overload_string_length;

private:
	void setOverload();
	bool shiftData( int offset);
	void parse( std::string & output);
};
