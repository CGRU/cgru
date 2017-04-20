#pragma once

#include "../include/afjob.h"

#include "../libafanasy/name_af.h"

class ParserHost
{

public:

	ParserHost( af::Service * i_service);
	~ParserHost();

	void read( const std::string & i_mode, std::string & output);

	inline int getPercent()           const { return m_percent;         }
	inline int getFrame()             const { return m_frame;           }
	inline int getPercentFrame()      const { return m_percentframe;    }
	inline int hasWarning()           const { return m_warning;         }
	inline int hasError()             const { return m_error;           }
	inline int isBadResult()          const { return m_badresult;       }
	inline int isFinishedSuccess()    const { return m_finishedsuccess; }
	inline std::string getActivity()  const { return m_activity;        }
	inline std::string getReport()    const { return m_report;          }
	inline char* getData( int *size ) const { *size = m_datasize; return m_data;}

private:
	af::Service * m_service;

	int  m_percent;
	int  m_frame;
	int  m_percentframe;
	bool m_error;
	bool m_warning;
	bool m_badresult;
	bool m_finishedsuccess;
	std::string m_activity;
	std::string m_report;

	char*              m_data;
	int                m_datasize;
	static const int   ms_DataSizeMax;
	static const int   ms_DataSizeHalf;
	static const int   ms_DataShiftMin;
	bool               m_overload;
	static const char* ms_overload_string;
	int                m_overload_string_length;

private:
	void parse( const std::string & i_mode, std::string & output);
};
