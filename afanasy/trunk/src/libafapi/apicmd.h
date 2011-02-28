#pragma once

#include <QtCore/QString>
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcafnodes.h"
#include "api.h"

namespace afapi
{
class Cmd
{
public:
   Cmd();
   ~Cmd();
/// Get joblist for all
   bool GetJobList();
/// Get joblist for user id
   bool GetJobListUserId(int userID);
/// Get job info from job Id
   bool GetJobInfo(int jobID);
/// Delete job from jobmask
   bool JobDelete(QString jobMask);
/// Get raw job data, to send to server socket ( call after successfull).
   char * getData();
/// Get raw job data length.
   int getDataLen();
private:
   af::Msg * message;
};
}
