#include "cmd_text.h"

#include <string.h>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdText::CmdText()
{
   setCmd("txt");
   setArgsCount(1);
   setInfo("Print text in server stdout.");
   setHelp("txt [text] Simple text message. For debug purposes.");
   setMsgType( af::Msg::TDATA);
}

CmdText::~CmdText(){}

bool CmdText::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   msg.setData( strlen(argv[0]), argv[0]);
   return true;
}

CmdTextGenerate::CmdTextGenerate()
{
   setCmd("txtgen");
   setArgsCount(2);
   setInfo("Generate text.");
   setHelp("txtgen [length] [first character] Generate length number of characters. For debug purposes.");
   setMsgType( af::Msg::TDATA);
}

CmdTextGenerate::~CmdTextGenerate(){}

bool CmdTextGenerate::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   int lenght = atoi(argv[0]);
   char* text = new char[lenght];
   char ch = '1';
   int linenum = 1;
   char linenumstr[128];
   for( int c = 0; c < lenght-1; c++)
   {
      text[c] = ch++;
      if( ch > 'z')
      {
         ch = '1';
         text[++c] = '\n';
         sprintf( linenumstr, "%d: ", linenum++ );
         int linenumlen = strlen( linenumstr);
         if( c+linenumlen+1 < lenght)
         {
            c++;
            memcpy( text+c, linenumstr, linenumlen-1);
            c += linenumlen-2;
         }
      }
   }
   text[lenght-2] = '\n';
   text[lenght-1] = '\0';
   text[0] = argv[1][0];
   msg.setData( lenght, text);
   if( Verbose) msg.stdOutData();
   return true;
}
