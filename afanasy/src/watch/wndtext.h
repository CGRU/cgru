#pragma once

#include "../libafanasy/msgclasses/mctaskoutput.h"
#include "../libafanasy/name_af.h"

#include "wnd.h"

class QVBoxLayout;
class QTextEdit;

class WndText : public Wnd
{
public:
	WndText( const QString & i_name);
	WndText( const QString & i_name, const std::string & i_str);
	WndText( const QString & i_name, af::Msg * i_msg);
	WndText( const af::MCTaskOutput & i_to);
	~WndText();

	void insertText( const char * i_text);
	void insertText( const QString & i_text);
	void insertText( const std::string & i_text);

protected:
	QVBoxLayout * layout;
	QTextEdit * qTextEdit;

private:
	void construct();
	void caseMsg( af::Msg * i_msg);
};
