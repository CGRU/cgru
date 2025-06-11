#ifdef WINNT
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "address.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "environment.h"

#ifdef WINNT
#define sprintf sprintf_s
#define sscanf sscanf_s
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

std::list<AddressMask> Address::ms_addr_masks;

Address::Address(int Port) : m_port(Port), m_family(Empty)
{
	// On cunstruction on client side:
	// there is no need to find out local host address.
	// Server will look at connected client address to register.
	memset(m_addr, 0, ms_addrdatalength);
}

Address::Address(const Address &other) { copy(other); }

Address::Address(const Address *other)
{
	if (other)
		copy(*other);
}

Address &Address::operator=(const Address &other)
{
	if (this != &other)
		copy(other);
	return *this;
}

void Address::copy(const Address &other)
{
	m_port = other.m_port;
	m_family = other.m_family;
	memcpy(&m_addr, &other.m_addr, ms_addrdatalength);
}

Address::Address(const struct sockaddr_storage *ss)
{
	memset(m_addr, 0, ms_addrdatalength);
	switch (ss->ss_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sa = (struct sockaddr_in *)ss;
			m_family = IPv4;
			m_port = ntohs(sa->sin_port);
			memcpy(m_addr, &(sa->sin_addr), 4);
			break;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *sa = (struct sockaddr_in6 *)ss;

			// Check for mapped IPv4 address: 1-80 bits == 0, 81-96 bits == 1 and last 32 bits == IPv4
			m_family = IPv4;
			char *data = (char *)&(sa->sin6_addr);
			for (int i = 0; i < 10; i++) // Check first 10 bytes for zeros:
				if (data[i] != char(0))
				{
					m_family = IPv6;
					break;
				}
			if (m_family == IPv4)
				for (int i = 10; i < 12; i++) // Check next 2 bytes for 0xFF:
					if (data[i] != char(0xFF))
					{
						m_family = IPv6;
						break;
					}

			if (m_family == IPv6)
			{
				m_port = ntohs(sa->sin6_port);
				memcpy(m_addr, &(sa->sin6_addr), 16);
			}
			else
			{
				struct sockaddr_in *sa = (struct sockaddr_in *)ss;
				m_port = ntohs(sa->sin_port);
				memcpy(m_addr, &(data[12]), 4);
			}
			break;
		}
		default: AFERROR("Address::Address: Unknown address family."); break;
	}
}

Address::~Address() {}

bool Address::equal(const af::Address &other) const
{
	if (false == equalIP(other))
		return false;
	if (m_port != other.m_port)
		return false;
	return true;
}

bool Address::equalIP(const af::Address &other) const
{
	if (isEmpty())
		return false;
	if (other.isEmpty())
		return false;
	if (m_family == other.m_family)
	{
		if (memcmp(&m_addr, &(other.m_addr), ms_addrdatalength) == 0)
			return true;
	}
	return false;
}

Address::Address(Msg *msg) { read(msg); }

void Address::v_readwrite(Msg *msg)
{
	rw_int8_t(m_family, msg);
	rw_uint16_t(m_port, msg);
	rw_data(m_addr, msg, ms_addrdatalength);
}

void Address::jsonWrite(std::ostringstream &o_str) const
{
	o_str << "{\"family\":";
	switch (m_family)
	{
		case IPv4:
		{
			o_str << "4";
			break;
		}
		case IPv6:
		{
			o_str << "6";
			break;
		}
		default: o_str << "0";
	}
	o_str << ",\"ip\":\"";
	generateIPStream(o_str, false);
	o_str << "\",\"port\":" << int(m_port) << "}";
}

void Address::jsonRead(const JSON &i_object)
{
	if (false == i_object.IsObject())
		return;

	clear();

	jr_int8("family", m_family, i_object);

	std::string ip;
	jr_string("ip", ip, i_object);
	setIP(ip);

	jr_uint16("port", m_port, i_object);
}

void Address::clear()
{
	m_port = 0, m_family = Empty;
	memset(m_addr, 0, ms_addrdatalength);
}

bool Address::setSocketAddress(struct sockaddr_storage *ss) const
{
	memset(ss, 0, sizeof(sockaddr_storage));
	switch (m_family)
	{
		case IPv4:
		{
			struct sockaddr_in *sa = (struct sockaddr_in *)ss;
			sa->sin_family = AF_INET;
			sa->sin_port = htons(m_port);
			memcpy(&(sa->sin_addr), m_addr, 4);
			break;
		}
		case IPv6:
		{
			struct sockaddr_in6 *sa = (struct sockaddr_in6 *)ss;
			sa->sin6_family = AF_INET6;
			sa->sin6_port = htons(m_port);
			memcpy(&(sa->sin6_addr), m_addr, 16);
			break;
		}
		case Empty: AFERROR("Address::setSocketAddress: Address is empty."); return false;
		default: AFERROR("Address::setSocketAddress: Unknown address family."); return false;
	}
	return true;
}

void Address::setIP(const af::Address &i_other)
{
	m_family = i_other.m_family;
	memcpy(&m_addr, &i_other.m_addr, ms_addrdatalength);
}

void Address::setIP(const std::string &i_str)
{
	if (i_str.empty())
		return;

	unsigned int ip6[8];
	int fields6 = sscanf(i_str.c_str(), "%x:%x:%x:%x:%x:%x:%x:%x", &(ip6[0]), &(ip6[1]), &(ip6[2]), &(ip6[3]),
						 &(ip6[4]), &(ip6[5]), &(ip6[6]), &(ip6[7]));
	if (fields6 == 8)
	{
		m_family = IPv6;
		for (int i = 0; i < 8; i++)
		{
			uint8_t byte0 = ip6[i] >> 8;
			uint8_t byte1 = ip6[i] - (byte0 << 8);
			m_addr[2 * i + 0] = byte0;
			m_addr[2 * i + 1] = byte1;
		}
		return;
	}

	unsigned int ip4[4];
	int fields4 = sscanf(i_str.c_str(), "%u.%u.%u.%u", &(ip4[0]), &(ip4[1]), &(ip4[2]), &(ip4[3]));
	if (fields4 == 4)
	{
		m_family = IPv4;
		for (int i = 0; i < 4; i++)
			m_addr[i] = ip4[i];
		return;
	}
	AFERRAR("Address::setIP: String \"%s\" is not an IP address.", i_str.c_str())
}

void Address::generateIPStream(std::ostringstream &stream, bool full) const
{
	switch (m_family)
	{
		case IPv4:
		{
			char buffer[64];
			sprintf(buffer, "%u.%u.%u.%u", uint8_t(m_addr[0]), uint8_t(m_addr[1]), uint8_t(m_addr[2]),
					uint8_t(m_addr[3]));
			stream << buffer;
			break;
		}
		case IPv6:
		{
			char buffer[64];
			for (int i = 0; i < 8; i++)
			{
				if (i != 0)
					stream << ':';
				uint8_t byte0 = uint8_t(m_addr[2 * i]);
				uint8_t byte1 = uint8_t(m_addr[2 * i + 1]);
				if (full)
				{
					if (byte0)
						sprintf(buffer, "%x%02x", byte0, byte1);
					else if (byte1)
						sprintf(buffer, "%x", byte1);
					if (byte0 || byte1)
						stream << buffer;
				}
				else
				{
					sprintf(buffer, "%02x%02x", byte0, byte1);
					stream << buffer;
				}
			}
			break;
		}
		case Empty:
			if (full)
				stream << "Empty address";
			break;
		default:
			if (full)
				stream << "Unknown address family";
			else
				stream << "AF=" << m_family;
			break;
	}
}

const std::string Address::generateIPString(bool full) const
{
	std::ostringstream stream;
	generateIPStream(stream, full);
	return stream.str();
}

void Address::generatePortStream(std::ostringstream &stream, bool full) const { stream << m_port; }

const std::string Address::generatePortString(bool full) const
{
	std::ostringstream stream;
	generatePortStream(stream, full);
	return stream.str();
}

void Address::v_generateInfoStream(std::ostringstream &stream, bool full) const
{
	if (full)
	{
		switch (m_family)
		{
			case IPv4: stream << "IPv4 Address: "; break;
			case IPv6: stream << "IPv6 Address: "; break;
		}
	}
	generateIPStream(stream, true);
	if (notEmpty() && (m_port != 0))
	{
		stream << ":";
		generatePortStream(stream, full);
	}
}

int Address::calcWeight() const
{
	int weight = sizeof(Address);
	return weight;
}

bool Address::readIpMask(const std::vector<std::string> &i_masks, bool i_verbose)
{
	ms_addr_masks.clear();

	for (std::vector<std::string>::const_iterator it = i_masks.begin(); it != i_masks.end(); it++)
	{
		int mask_len = 0;
		char mask_bytes[ms_addrdatalength];
		Address::Family mask_family = Address::Empty;

		if (((*it).find('.') != std::string::npos) && ((*it).find(':') != std::string::npos))
		{
			AFERRAR("Invalid Server IP Mask: '%s' - and '.' and ':' characters preset.", (*it).c_str());
			return false;
		}
		if ((*it).find('.') != std::string::npos)
		{
			// IPv4
			mask_family = Address::IPv4;

			std::vector<std::string> byte_strs = strSplit(*it, ".");
			if ((byte_strs.size() < 2) || (byte_strs.size() > 4))
			{
				AFERRAR("Invalid Server IPv4 Mask: '%s' - should be 2 - 4 entries separated with '.'",
						(*it).c_str());
				return false;
			}

			for (int i = 0; i < byte_strs.size(); i++)
			{
				if (byte_strs[i] == "*")
				{
					break;
				}
				bool ok;
				unsigned byte = (unsigned)af::stoi(byte_strs[i], &ok);
				if (false == ok)
				{
					AFERRAR("Invalid Server IP Mask: '%s': Invalid decimal number '%s'.", (*it).c_str(),
							byte_strs[i].c_str());
					return false;
				}
				if (byte > 0xff)
				{
					AFERRAR("Invalid Server IP Mask: '%s': Too big decimal number '%s'.", (*it).c_str(),
							byte_strs[i].c_str());
					return false;
				}
				mask_bytes[mask_len] = byte;
				mask_len++;
			}
		}
		else if ((*it).find(':') != std::string::npos)
		{
			// IPv6
			mask_family = Address::IPv6;

			std::vector<std::string> byte_strs = strSplit(*it, ":");
			if ((byte_strs.size() < 2) || (byte_strs.size() > 8))
			{
				AFERRAR("Invalid Server IPv6 Mask: '%s' - should be 2 - 8 entries separated with ':'",
						(*it).c_str());
				return false;
			}

			for (int i = 0; i < byte_strs.size(); i++)
			{
				if (byte_strs[i] == "*")
				{
					break;
				}

				unsigned byte;
				sscanf(byte_strs[i].c_str(), "%x", &byte);
				uint8_t hi = byte >> 8;
				uint8_t lo = byte - (int(hi) << 8);
				mask_bytes[mask_len++] = hi;
				mask_bytes[mask_len++] = lo;
			}
		}
		else
		{
			AFERRAR("Invalid Server IP Mask: '%s' - no '.' or ':' characters preset.", (*it).c_str());
			return false;
		}

		ms_addr_masks.push_back(AddressMask(mask_len, mask_bytes, mask_family, i_verbose));
	}
	return true;
}

bool Address::matchIpMask() const
{
	AFINFA("Address::matchIpMask: %s", v_generateInfoString().c_str())

	if (ms_addr_masks.size() == 0)
	{
		// No masks exists - any address allowed
		return true;
	}

	for (std::list<AddressMask>::const_iterator it = ms_addr_masks.begin(); it != ms_addr_masks.end(); it++)
	{
		if (m_family != it->m_family)
		{
			// Skip if family not equal
			continue;
		}

		if (it->m_len == 0)
		{
			// Mask has no length - any address match
			return true;
		}

		if (memcmp(m_addr, it->m_bytes, it->m_len) == 0)
		{
			// Return that mask is matched
			return true;
		}
	}

	// Return that mask is NOT matched
	return false;
}

AddressMask::AddressMask(int i_len, const char *i_bytes, Address::Family i_family, bool i_verbose)
	: m_len(i_len), m_family(i_family)
{
	if (m_len > 0)
	{
		memcpy(m_bytes, i_bytes, i_len);
	}

	if (false == i_verbose)
		return;

	// Print mask:
	printf("Server Mask");

	switch (m_family)
	{
		case Address::IPv4:
		{
			printf(" IPv4: ");
			if (m_len == 0)
				printf("any");
			for (int i = 0; i < m_len; i++)
			{
				if (i)
					printf(".");
				uint8_t byte = m_bytes[i];
				printf("%u", byte);
			}
			break;
		}
		case Address::IPv6:
		{
			printf(" IPv6: ");
			if (m_len == 0)
				printf("any");
			for (int i = 0; i < m_len; i++)
			{
				if ((i > 0) && (i % 2 == 0))
					printf(":");
				uint8_t byte = m_bytes[i];
				printf("%02x", byte);
			}
			break;
		}
		default: break; // Do nothing
	}

	printf("\n");
}
