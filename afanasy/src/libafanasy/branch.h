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
	branch.h - Describes a job branch class.
	Branch can contain jobs and other branches. Branch is designed to control a pack of jobs.
*/

#pragma once

#include "afwork.h"

namespace af
{
class Branch: public Work
{
public:

	Branch(const std::string & i_path);

	Branch(Msg * msg);

	// To construct from store:
	Branch(int i_id = 0);

	virtual ~Branch();

	inline const std::string & getParentPath() const {return m_parent_path;}

	void v_generateInfoStream(std::ostringstream & stream, bool full = false) const;

	virtual void v_jsonWrite(std::ostringstream & o_str, int i_type) const;

	bool jsonRead(const JSON & i_object, std::string * io_changes = NULL);

	static const std::string FilterPath(const std::string & i_path);

protected:

	std::string m_parent_path;

	int64_t m_time_creation;
	int64_t m_time_empty;
	int32_t m_branches_num;


private:

	void v_readwrite(Msg * msg);


private:

};
}

