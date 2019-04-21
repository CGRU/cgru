/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	af::Farm is class for storing farm properties for af::Render and af::Pool
*/

#include "affarm.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace af;

Farm::Farm()
{
}

Farm::~Farm()
{
}

void Farm::readwrite(Msg *msg)
{
	rw_StringVect(m_services,          msg);
	rw_StringVect(m_services_disabled, msg);
}

void Farm::jsonRead(const JSON &i_object, std::string *io_changes)
{
	jr_stringvec("services",          m_services,          i_object);
	jr_stringvec("services_disabled", m_services_disabled, i_object);
}

void Farm::jsonWrite(std::ostringstream &o_str, int i_type) const
{
	if (m_services.size())
	{
		o_str << ",\n\"services\":[";
		for (int i = 0; i < m_services.size(); i++)
		{
			if (i) o_str << ",";
			o_str << '\"' << m_services[i] << '\"';
		}
		o_str << ']';
	}

	if (m_services_disabled.size())
	{
		o_str << ",\n\"services_disabled\":[";
		for (int i = 0; i < m_services_disabled.size(); i++)
		{
			if (i) o_str << ",";
			o_str << '\"' << m_services_disabled[i] << '\"';
		}
		o_str << ']';
	}
}

int Farm::calcWeight() const
{
	int weight = sizeof(Farm);

	return weight;
}
