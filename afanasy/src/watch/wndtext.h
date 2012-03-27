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

   void insertText( const QString text);

protected:
   QVBoxLayout * layout;
   QTextEdit * qTextEdit;
};
