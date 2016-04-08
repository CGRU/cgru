#pragma once

#include "../libafanasy/msg.h"

extern bool Verbose;
extern bool Help;
extern int Protocol;
extern std::string ServerName;
extern int ServerPort;

class Cmd
{
public:
	Cmd();
	virtual ~Cmd();

	inline void setCmd(   const std::string & str) { m_cmd    = str;   }
	inline void setInfo(  const std::string & str) { m_info   = str;   }
	inline void setHelp(  const std::string & str) { m_help   = str;   }
	inline void setArgsCount(  int value         ) { m_argscount   = value; }
	inline void setMsgType(    int value         ) { m_msgtype     = value; }
	inline void setMsgOutType( int value         ) { m_msgouttype  = value; }

	inline bool isCmd( const std::string & str)  const { return m_cmd == str;          }
	inline bool hasArgsCount( int value )        const { return m_argscount  <= value; }
	inline bool isMsgOutType( int value )        const { return m_msgouttype == value; }

	inline const std::string & getCmd()  const { return m_cmd;  }
	inline const std::string & getInfo() const { return m_info; }
	inline const std::string & getHelp() const { return m_help; }
	inline int getMsgType()    const { return m_msgtype;    }
	inline int getArgsCount()  const { return m_argscount;  }
	inline int getMsgOutType() const { return m_msgouttype; }

/// Generate message parsing command line arguments. Return \c true on sucsess.
	virtual bool v_processArguments( int argc, char** argv, af::Msg &msg) = 0;
/// Print message information in stdout.
	virtual void v_msgOut( af::Msg& msg);

	virtual void v_printInfo() const;
	virtual void v_printHelp() const;

	inline const std::string getStreamString() const { return m_str.str(); }

protected:
	std::ostringstream m_str;

private:

	int  m_msgtype;
	int  m_msgouttype;
	int  m_argscount;

	std::string m_cmd;
	std::string m_info;
	std::string m_help;
};
