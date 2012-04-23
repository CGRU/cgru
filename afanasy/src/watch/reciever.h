#pragma once

#include "../libafanasy/name_af.h"

#include <QtGui/QTextEdit>

class Reciever
{
public:
   Reciever();
   virtual ~Reciever();

   virtual bool caseMessage( af::Msg * msg) = 0;

   virtual void connectionLost();
   virtual void connectionEstablished();

private:
};
