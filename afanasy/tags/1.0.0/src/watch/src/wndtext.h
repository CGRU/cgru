#pragma once

#include "wnd.h"

#include <name_af.h>

class QVBoxLayout;
class QTextEdit;

class WndText : public Wnd
{
public:
   WndText( const QString & Name, af::Msg * msg = NULL);
   ~WndText();

protected:
   QVBoxLayout * layout;
   QTextEdit * qTextEdit;
};
