#pragma once

#include "msgclass.h"
#include "mctaskup.h"

namespace af
{

class MCTaskPos : public MsgClass
{
public:
	MCTaskPos( int job_id = 0, int block_num = 0, int task_num = 0, int Number = 0);
	MCTaskPos( const MCTaskUp & i_tup);
	MCTaskPos( Msg * msg);
	~MCTaskPos();

	/// If all 4 fields are euqal (same task in the job + aux number)
	bool isEqual( const MCTaskPos & i_other) const;

	/// If first 3 fields are euqal (same task in the job)
	bool isSameTask( const MCTaskPos & i_other) const;

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

	inline int getJobId()    const { return jobid;    }
	inline int getBlockNum() const { return blocknum; }
	inline int getTaskNum()  const { return tasknum;  }
	inline int getNumber()   const { return number;   }

private:
	int32_t jobid;
	int32_t blocknum;
	int32_t tasknum;

	// Used to indentify muilti-host task exec
	int32_t number;

public:
	void v_readwrite( Msg * msg);
	void jsonWrite( std::ostringstream & o_str) const;
};
}
