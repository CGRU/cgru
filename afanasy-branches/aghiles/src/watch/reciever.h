#pragma once

#include <QtGui/QTextEdit>

#include "../libafanasy/name_af.h"

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
