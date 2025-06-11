#ifndef AF_PASSWD_H
#define AF_PASSWD_H

#include <string>

#include "md5.h"

namespace af
{
class Passwd
{
  public:
	Passwd();
	Passwd(const std::string &i_visor_key, const std::string &i_god_key);
	~Passwd();

	/// Check current key matching password sequence.
	bool checkKey(const char i_key, bool &o_visor_mode, bool &o_god_mode);

	bool checkPassVisor(const std::string &i_pass);
	bool checkPassGOD(const std::string &i_pass);

	static void PrintKey(unsigned char *i_key);

	static const int ms_digest_len = 16;
	static const int ms_key_len = 32;
	static const int ms_pswd_len = 5;

  private:
	unsigned char m_visor[ms_key_len];
	unsigned char m_god[ms_key_len];
	unsigned char m_characters[ms_pswd_len];
	unsigned char m_buffer[ms_key_len];

  private:
	void init();
	void calculate(unsigned char *i_key, const unsigned char *i_passwd, int i_len);

  private:
	MD5_CTX m_context;
	unsigned char m_digest[ms_digest_len];
};
} // namespace af
#endif
