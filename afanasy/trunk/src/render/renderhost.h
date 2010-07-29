#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/render.h"

#include "../libafqt/name_afqt.h"

class QTcpSocket;

class Parser;
class PyRes;

afqt::QMsg* update_handler_ptr( afqt::QMsg * msg);

class RenderHost: public af::Render
{
public:
   RenderHost( int32_t State, uint8_t Priority);
   RenderHost();
   ~RenderHost();

   inline void setId( int new_id) { id = new_id;}

   afqt::QMsg* updateMsg( afqt::QMsg *msg);

private:
#ifdef WINNT
   QStringList windowsmustdie;
#endif
   std::vector<PyRes*> pyres;
   afqt::QMsg * upmsg;
};
