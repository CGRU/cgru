#ifndef AF_PASSWD_H
#define AF_PASSWD_H

#include "md5.h"

#include <QtCore/QString>

namespace af
{
class Passwd
{
public:
   Passwd();
   Passwd( const QString & VisorKey, const QString & GodKey);
   ~Passwd();

/// Check current key matching password sequence.
   bool checkKey( const char key, bool & VisorMode, bool & GodMode);

   void calculate( unsigned char * key, const char * passwd);

   static void PrintKey( unsigned char * key);

   static const int digest_len = 16;
   static const int key_len = 32;
   static const int pswd_len = 5;

private:
   unsigned char visor[key_len];
   unsigned char   god[key_len];
   unsigned char characters[pswd_len];
   unsigned char buffer[key_len];

private:
   void init();
   void calculate( unsigned char * key, unsigned char * passwd, int len);

private:
   MD5_CTX context;
   unsigned char digest[digest_len];
};
}
#endif
