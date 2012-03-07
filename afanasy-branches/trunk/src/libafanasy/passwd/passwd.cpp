#include "passwd.h"

#include <stdio.h>
#include <string.h>

#ifdef WINNT
#define spritnf spritnf_s
#endif

using namespace af;

Passwd::Passwd()
{
   init();
}

Passwd::Passwd( const std::string & VisorKey, const std::string & GodKey)
{
   init();
   std::string visor_str = VisorKey;
   std::string   god_str = GodKey;
   if( visor_str.size() > key_len) visor_str.resize(key_len);
   if(   god_str.size() > key_len)   god_str.resize(key_len);

   strncpy( (char*) visor, visor_str.c_str(), visor_str.size());
   strncpy( (char*)   god,   god_str.c_str(),   god_str.size());
//write(1,visor,key_len); write(1,"\n",1);
//write(1,  god,key_len); write(1,"\n",1);
   char ch1, ch2;
   for( int d = 0; d < digest_len; d++)
   {
      ch1 = visor[d*2];
      ch2 = visor[d*2+1];
      ch1 = ((ch1 <= '9') ? (ch1 - '0') : (ch1 - 'a'+10));
      ch2 = ((ch2 <= '9') ? (ch2 - '0') : (ch2 - 'a'+10));
      visor[d] = 16*ch1 + ch2;
      ch1 = god[d*2];
      ch2 = god[d*2+1];
      ch1 = ((ch1 <= '9') ? (ch1 - '0') : (ch1 - 'a'+10));
      ch2 = ((ch2 <= '9') ? (ch2 - '0') : (ch2 - 'a'+10));
      god[d] = 16*ch1 + ch2;
   }
//   Passwd::PrintKey( visor);
//   Passwd::PrintKey( god );
}

void Passwd::init()
{
   for( int i = 0; i < pswd_len; i++) characters[i] = '_';
   for( int i = 0; i < key_len; i++)
   {
       visor[i] = '0';
         god[i] = '0';
      buffer[i] = '\0';
   }
}

Passwd::~Passwd()
{
}

bool Passwd::checkKey( const char key, bool & VisorMode, bool & GodMode)
{
   for( int i = 1; i < pswd_len; i++) characters[i-1] = characters[i];
   characters[pswd_len-1] = key;


   calculate( digest, characters, pswd_len);

//write(1,characters,pswd_len); write(1,"\n",1);
//Passwd::PrintKey( digest);

   if( memcmp( visor, digest, digest_len) == 0)
   {
      if( VisorMode)
      {
         VisorMode  = false;
         printf("VISOR MODE OFF\n");
         if( GodMode) printf("GOD MODE OFF\n");
         GodMode = false;
      }
      else
      {
         VisorMode  = true;
         printf("VISOR MODE ON\n");
         if( GodMode) printf("GOD MODE OFF\n");
         GodMode = false;
      }
      return true;
   }
   else if( memcmp( god, digest, digest_len) == 0)
   {
      if( GodMode)
      {
         GodMode = false;
         printf("GOD MODE OFF\n");
         if( VisorMode) printf("VISOR MODE OFF\n");
         VisorMode  = false;
      }
      else
      {
         GodMode  = true;
         printf("GOD MODE ON\n");
         if( !VisorMode) printf("VISOR MODE ON\n");
         VisorMode  = true;
      }
      return true;
   }

   return false;
}

void Passwd::calculate( unsigned char * key, const char * passwd)
{
   int len = strlen( passwd);
   calculate( key, (unsigned char *)passwd, len);
}

void Passwd::PrintKey( unsigned char * key)
{
   for( int i = 0; i < Passwd::digest_len; i++) printf ("%02x", key[i]);
   printf ("\n");
}

void Passwd::calculate( unsigned char * key, unsigned char * passwd, int len)
{
   MD5Init   ( &context);
   MD5Update ( &context, passwd, len);
   MD5Final  ( key, &context);
}
