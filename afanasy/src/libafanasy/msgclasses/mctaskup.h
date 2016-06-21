#pragma once

#include "../name_af.h"

#include "msgclass.h"

namespace af
{

class MCTaskUp : public MsgClass
{
public:
	MCTaskUp
		(
			int i_clientid,

			int i_numjob,
			int i_numblock,
			int i_numtask,
			int i_number = -1,

			int i_status        = -1,
			int i_percent       = -1,
			int i_frame         = -1,
			int i_percent_frame = -1,
			const std::string & i_log      = std::string(),
			const std::string & i_activity = std::string(),
			const std::string & i_report   = std::string(),
			const std::string & i_listened = std::string(),
			int i_datalen = 0,
			char * i_data = NULL
		);
	MCTaskUp( Msg * msg);
	~MCTaskUp();

	bool isSameTask( const MCTaskUp & i_other) const;

	inline int getClientId()                 const { return m_clientid;      }

	inline int getNumJob()                   const { return m_numjob;        }
	inline int getNumBlock()                 const { return m_numblock;      }
	inline int getNumTask()                  const { return m_numtask;       }
	inline int getNumber()                   const { return m_number;        }

	inline int getStatus()                   const { return m_status;        }
	inline int getPercent()                  const { return m_percent;       }
	inline int getFrame()                    const { return m_frame;         }
	inline int getPercentFrame()             const { return m_percent_frame; }
  	inline const std::string & getActivity() const { return m_activity;      }
  	inline const std::string & getReport()   const { return m_report;        }
  	inline const std::string & getLog()      const { return m_log;           }
	inline int getDataLen()                  const { return m_datalen;       }
	inline const char * getData()            const { return m_data;          }

	inline bool hasListened()                const { return m_listened.size(); }
	inline const std::string & getListened() const { return m_listened;        }

	inline void setParsedFiles( const std::vector<std::string> & i_files) { m_parsed_files = i_files; }
	inline const std::vector<std::string> & getParsedFiles() const { return m_parsed_files; }

	inline int getFilesNum() const { return m_files_num; }
	inline int getFileSize( int i_num) const { return m_files_sizes[i_num]; }
	inline const std::string & getFileName( int i_num) const { return m_files_names[i_num]; }
	const char * getFileData( int i_num) const;

	void addFile( const std::string & i_name, const char * i_data, int i_size);

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

private:
	int32_t m_clientid;

	int32_t m_numjob;
	int32_t m_numblock;
	int32_t m_numtask;

	/// Used to indentify multi-host tasks:
	int32_t m_number;

	int8_t  m_status;
	int8_t  m_percent;
	int32_t m_frame;
	int8_t  m_percent_frame;

	std::string m_activity;
	std::string m_report;
	std::string m_log;

	std::string m_listened;

	int32_t m_datalen;
	char * m_data;

	std::vector<std::string> m_parsed_files;

	int32_t m_files_num;
	std::vector<std::string> m_files_names;
	std::vector<int32_t> m_files_sizes;
	char * m_files_data;
	int m_files_data_len;
	int m_files_data_buflen;

	bool m_deleteData;

private:
	void v_readwrite( Msg * msg);

	void rwData( Msg * msg);
	void rwFiles( Msg * msg);
};
}
