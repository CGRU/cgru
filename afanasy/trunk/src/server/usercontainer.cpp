#include "usercontainer.h"

#include <stdio.h>
#include <string.h>

#include "../include/afanasy.h"

#include "../libafsql/qdbconnection.h"

#include "afcommon.h"
#include "msgaf.h"
#include "renderaf.h"
#include "monitorcontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

UserContainer::UserContainer():
   AfContainer( AFUSER::MAXCOUNT),
   currentUserId( 0),
   currentPriority( -1)
{
}

UserContainer::~UserContainer()
{
AFINFO("UserContainer::~UserContainer:\n");
}

UserAf* UserContainer::addUser( const QString &username, const QString &hostname, MonitorContainer * monitoring)
{
   {
      UserContainerIt usersIt( this);
      for(UserAf *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
      {
         if( username == user->getName())
         {
            if( user->getHostName() != hostname)
            {
               user->setHostName( hostname);
               if( monitoring) monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());
            }
            return user;
         }
      }
   }

   UserAf *user = new UserAf( username, hostname);
   if( add((af::Node*)user) == 0)
   {
      AFERROR("UserContainer::addUser: Can't add user to container.\n");
      delete user;
      return NULL;
   }

   if( monitoring) monitoring->addEvent( af::Msg::TMonitorUsersAdd, user->getId());

   printTime(); printf(" : User registered: "); user->stdOut( false );
   return user;
}

void UserContainer::addUser( UserAf * user)
{
   add((af::Node*)user);
}

void UserContainer::setPermanent( const af::MCGeneral & usr, bool permanent, MonitorContainer * monitoring)
{
   QString username( usr.getName());
   QString hostname( usr.getHostName());
printf("UserContainer::setPermanent: %s@%s - %d\n", username.toUtf8().data(), hostname.toUtf8().data(), permanent);
   //
   // Try to find user with provided name in container to edit permanent property
   //
   UserContainerIt usersIt( this);
   for(UserAf *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
   {
      if( username == user->getName())
      {
         bool changed = false;
         if( user->getHostName() != hostname )
         {
            user->setHostName( hostname);
            changed = true;
         }

         // set permanent property if it differ
         if( user->isPermanent() != permanent)
         {
            changed = true;
            user->setPermanent( permanent);
            if( permanent) AFCommon::QueueDBAddItem( user);
            else AFCommon::QueueDBDelItem( user);
         }

         if( changed && monitoring) monitoring->addEvent( af::Msg::TMonitorUsersChanged, user->getId());

         // return if user exists in container
         return;
      }
   }
   //
   //    User with provided name does not exist container
   //
   if( permanent == false)
   {
      // Error: user to unset permanent does not exist
      AFERRAR("UserContainer::setPermanent: No user \"%s\" founded.\n", username.toUtf8().data());
      return;
   }
   //
   //    Create a new permanent user and put him to database
   //
   UserAf *user = new UserAf( username, hostname);
   if( add((af::Node*)user) == 0)
   {
      delete user;
      AFERRAR("UserContainer::setPermanent: Can't add user \"%s\" node to container.", username.toUtf8().data());
      return;
   }
   user->setPermanent( true);
   AFCommon::QueueDBAddItem( user);
   if( monitoring) monitoring->addEvent( af::Msg::TMonitorUsersAdd, user->getId());
   return;
}

bool UserContainer::genTask( RenderAf *render, MonitorContainer * monitoring)
{
//printf("\nUserContainer::genTask: render - %s\n", render->getName().toUtf8().data());
   UserContainerIt usersIt( this);        // initialize users iterator
   QList<UserAf*> users;

   //
   // Calculate all users need value and push them into the list
   for( UserAf *user = usersIt.user(); user != NULL; usersIt.next(), user = usersIt.user())
      if( user->canRun( render)) users.push_back(user);

   int count = users.count();

   //
   // Sort users list by need value
   for( int pos = count; pos > 1; pos--)
      for( int u = 1; u < pos; u++)
         if( (users[u-1]->getNeed()) < (users[u]->getNeed()) )
            users.swap( u-1, u);

//printf("\n");for(int u=0;u<count;u++){printf("%s - %g\n",users[u]->getName().toUtf8().data(),users[u]->getNeed());}printf("\n");

   //
   // Ask user with most need value to generate a task
   bool  firstlap = true;
   int   userlastneed = -1;
   int   userfirstneed = -1;
   float prevneed = -1;
   for( int u = 0; u < count; u++)
   {
//printf("\nUserContainer::genTask: trying - %s - %g (u=%d)\n", users[u]->getName().toUtf8().data(), users[u]->getNeed(), u);
/*      if( render->isNimby())    // If owner captured a render
      {
         if( users[u]->getName() != render->getUserName() ) continue; // Skip if user is not render owner
      }*/
      float need = users[u]->getNeed();                        // Get user need
      if( need != prevneed)                                    // If current need has new value
      {
//printf("UserContainer::genTask: this is first lap for need = %g ( pevious need = %g)\n", need, prevneed);
         firstlap = true;                                      // This is first lap
         prevneed = need;                                      // Store need value
      }
      if( users[u]->isSolved() )                               // If user is sloved
      {
//printf("UserContainer::genTask: user is solved - %s - %g (u=%d)\n", users[u]->getName().toUtf8().data(), need, u);
         int usameneed = u + 1;                                // Get next user index
         bool shift = false;                                   // Whether shift to next unsolved user with the same need
         for( ; usameneed < count; usameneed++)                // Search for unsolved user with the same need
         {
            if( need != users[usameneed]->getNeed() ) break;   // No more users with the same need
            if( users[usameneed]->isSolved() ) continue;       // Search next unsolved user
            shift = true;                                      // This is unsolved user with the same need
//printf("UserContainer::genTask: shifting - %s ( u=%d -> %d )\n", users[usameneed]->getName().toUtf8().data(), u, usameneed);
            break;
         }
         if( shift ) u = usameneed;                            // Unsolved user with the same need founded
         else
         {
//printf("UserContainer::genTask: reset solving for need = %g\n", need);
            for( usameneed = u; usameneed < count; usameneed++)// Set users with the same need to unsolved
            {
               if( need != users[usameneed]->getNeed() ) break;
               users[usameneed]->setSolved( false);
            }
         }
      }
      users[u]->setSolved( true);                              // Set user solved
//printf("UserContainer::genTask: try to generate - %s (u=%d)\n", users[u]->getName().toUtf8().data(), u);
      if( users[u]->genTask( render, monitoring))                          // Generate user task if any
      {
//printf("UserContainer::genTask: generated - %s (u=%d)\n\n\n", users[u]->getName().toUtf8().data(), u);
         users[u]->calcNeed();
         return true;
      }
      else
      {
//printf("UserContainer::genTask: not generated - %s (u=%d)\n\n\n", users[u]->getName().toUtf8().data(), u);
         if( u < count-1)                                      // If we have some more users
         {
            if( users[u+1]->getNeed() != need)                 // If next need has a new value
            {                                                  // and we are on the first lap
               if( firstlap )                                  // we run the second lap
               {
//printf("UserContainer::genTask: this is last user for first lap for need = %g\n", need);
                  userlastneed = u;                            // This is the last user for current need
                  userfirstneed = u;
                  for( ; u >= 0; u-- ) //for( ; u--; u >= 0 )
                  {
                     if( users[u]->getNeed() != need ) break;
                     userfirstneed = u;
                  }
                  for( u = userfirstneed; u < userlastneed; u++) users[u]->setSolved( false);
                  u = userfirstneed-1;
                  firstlap = false;
//printf("UserContainer::genTask: second lap: userfirstneed = %d, userlastneed %d\n", userfirstneed, userlastneed);
               }
            }
         }
      }
   }
//printf("UserContainer::genTask: no task generated for render - %s\n\n\n\n\n", render->getName().toUtf8().data());
   return false;
}

MsgAf* UserContainer::generateJobsList( int id)
{
   UserContainerIt usersIt( this);
   UserAf* user = usersIt.getUser( id);
   if( user == NULL) return NULL;

   MCAfNodes mcjobs;
   user->jobsinfo( mcjobs);

   return new MsgAf( Msg::TJobsList, &mcjobs);
}

MsgAf* UserContainer::generateJobsList( const af::MCGeneral & ids)
{
   UserContainerIt usersIt( this);
   MCAfNodes mcjobs;

   int count = ids.getCount();
   for( int i = 0; i < count; i++)
   {
      UserAf* user = usersIt.getUser( ids.getId(i));
      if( user == NULL) continue;
      user->jobsinfo( mcjobs);
   }

   return new MsgAf( Msg::TJobsList, &mcjobs);
}

//############################################################################
//                               UserContainerIt
//############################################################################

UserContainerIt::UserContainerIt( UserContainer* userslist, bool skipZombies):
   AfContainerIt( (AfContainer*)userslist, skipZombies)
{
}

UserContainerIt::~UserContainerIt()
{
}
