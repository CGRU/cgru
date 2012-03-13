#pragma once

#include "../libafanasy/name_af.h"

#include "wnd.h"

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
