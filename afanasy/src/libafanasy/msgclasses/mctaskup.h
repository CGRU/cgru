#pragma once

#include "../name_af.h"

#include "msgclass.h"

namespace af
{

class MCTaskUp : public MsgClass
{
public:
	MCTaskUp();
	MCTaskUp
		(
			int clientId,

			int numJob,
			int numBlock,
			int numTask,
			int number,

			int UpStatus,
			int UpPercent      = -1,
			int UpFrame        = -1,
			int UpPercentFrame = -1,
			std::string activity = "",
			int dataLen        = 0,
			char * data        = NULL
		);
	MCTaskUp( Msg * msg);
	~MCTaskUp();

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
	inline int getDataLen()                  const { return m_datalen;       }
	inline const char * getData()            const { return m_data;          }

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

private:
	int32_t m_clientid;

	int32_t m_numjob;
	int32_t m_numblock;
	int32_t m_numtask;
	int32_t m_number;

	int8_t  m_status;
	int8_t  m_percent;
	int32_t m_frame;
	int8_t  m_percent_frame;
	std::string m_activity;
	int32_t m_datalen;

	char *m_data;

	bool m_deleteData;

private:
	void v_readwrite( Msg * msg);
};
}
