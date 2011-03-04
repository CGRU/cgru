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

   inline void setRecieving() { recieving = true; }
   inline void setCmd(   const std::string & str) { cmd    = str;   }
   inline void setInfo(  const std::string & str) { info   = str;   }
   inline void setHelp(  const std::string & str) { help   = str;   }
   inline void setArgsCount(  int value         ) { argscount   = value; }
   inline void setMsgType(    int value         ) { msgtype     = value; }
   inline void setMsgOutType( int value         ) { msgouttype  = value; }

   inline bool isRecieving()                    const { return recieving;           }
   inline bool isCmd( const std::string & str)  const { return cmd == str;          }
   inline bool hasArgsCount( int value )        const { return argscount  <= value; }
   inline bool isMsgOutType( int value )        const { return msgouttype == value; }

   inline const std::string & getCmd()  const { return cmd;  }
   inline const std::string & getInfo() const { return info; }
   inline const std::string & getHelp() const { return help; }
   inline int getMsgType()    const { return msgtype;    }
   inline int getArgsCount()  const { return argscount;  }
   inline int getMsgOutType() const { return msgouttype; }

/// Generate message parsing command line arguments. Return \c true on sucsess.
   virtual bool processArguments( int argc, char** argv, af::Msg &msg) = 0;
/// Print message information in stdout.
   virtual void msgOut( af::Msg& msg);

   virtual void printInfo() const;
   virtual void printHelp() const;

private:

   int  msgtype;
   int  msgouttype;
   int  argscount;
   bool recieving;

   std::string cmd;
   std::string info;
   std::string help;
};
