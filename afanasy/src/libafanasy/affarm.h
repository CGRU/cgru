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
	affarm.h - Afanasy farm properies.
	af::Farm is class for storing farm properties for af::Render and af::Pool
*/

#include "../include/afanasy.h"

#include "af.h"

#pragma once

namespace af
{
class Farm
{
public:
	Farm();
	virtual ~Farm();

	void jsonRead(const JSON &i_object, std::string *io_changes = NULL);
	void jsonWrite(std::ostringstream &o_str, int i_type) const;

	friend class AfFarmSrv;

protected:
	void readwrite(Msg *msg); ///< Read or write node attributes in message

	int calcWeight() const; ///< Calculate and return memory size.

public:
	std::vector<std::string> m_services;
	std::vector<std::string> m_services_disabled;
};
}
