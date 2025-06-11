#include "passwd.h"

#include <stdio.h>
#include <string.h>

#ifdef WINNT
#define spritnf spritnf_s
#endif

using namespace af;

Passwd::Passwd() { init(); }

Passwd::Passwd(const std::string &i_visor_key, const std::string &i_god_key)
{
	init();
	std::string visor_str = i_visor_key;
	std::string god_str = i_god_key;
	if (visor_str.size() > ms_key_len)
		visor_str.resize(ms_key_len);
	if (god_str.size() > ms_key_len)
		god_str.resize(ms_key_len);

	strncpy((char *)m_visor, visor_str.c_str(), visor_str.size());
	strncpy((char *)m_god, god_str.c_str(), god_str.size());
	char ch1, ch2;
	for (int d = 0; d < ms_digest_len; d++)
	{
		ch1 = m_visor[d * 2];
		ch2 = m_visor[d * 2 + 1];
		ch1 = ((ch1 <= '9') ? (ch1 - '0') : (ch1 - 'a' + 10));
		ch2 = ((ch2 <= '9') ? (ch2 - '0') : (ch2 - 'a' + 10));
		m_visor[d] = 16 * ch1 + ch2;
		ch1 = m_god[d * 2];
		ch2 = m_god[d * 2 + 1];
		ch1 = ((ch1 <= '9') ? (ch1 - '0') : (ch1 - 'a' + 10));
		ch2 = ((ch2 <= '9') ? (ch2 - '0') : (ch2 - 'a' + 10));
		m_god[d] = 16 * ch1 + ch2;
	}
}

void Passwd::init()
{
	for (int i = 0; i < ms_pswd_len; i++)
		m_characters[i] = '_';
	for (int i = 0; i < ms_key_len; i++)
	{
		m_visor[i] = '0';
		m_god[i] = '0';
		m_buffer[i] = '\0';
	}
}

Passwd::~Passwd() {}

bool Passwd::checkKey(const char i_key, bool &o_visor_mode, bool &o_god_mode)
{
	o_visor_mode = false;
	o_god_mode = false;

	for (int i = 1; i < ms_pswd_len; i++)
		m_characters[i - 1] = m_characters[i];
	m_characters[ms_pswd_len - 1] = i_key;

	calculate(m_digest, m_characters, ms_pswd_len);

	if (memcmp(m_visor, m_digest, ms_digest_len) == 0)
	{
		o_visor_mode = true;
		return true;
	}

	if (memcmp(m_god, m_digest, ms_digest_len) == 0)
	{
		o_god_mode = true;
		return true;
	}

	return false;
}

bool Passwd::checkPassVisor(const std::string &i_pass)
{
	calculate(m_digest, (const unsigned char *)(i_pass.c_str()), i_pass.length());

	return memcmp(m_visor, m_digest, ms_digest_len) == 0;
}

bool Passwd::checkPassGOD(const std::string &i_pass)
{
	calculate(m_digest, (const unsigned char *)(i_pass.c_str()), i_pass.length());

	return memcmp(m_god, m_digest, ms_digest_len) == 0;
}

void Passwd::PrintKey(unsigned char *i_key)
{
	for (int i = 0; i < Passwd::ms_digest_len; i++)
		printf("%02x", i_key[i]);
	printf("\n");
}

void Passwd::calculate(unsigned char *i_key, const unsigned char *i_passwd, int i_len)
{
	MD5Init(&m_context);
	MD5Update(&m_context, i_passwd, i_len);
	MD5Final(i_key, &m_context);
}
