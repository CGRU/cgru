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

#pragma once

#include <istream>
#include <string>
#include <vector>

namespace af
{
bool platformParseOsReleaseLine(const std::string & i_line, std::string & o_key, std::string & o_value);
bool platformParseOsRelease(std::istream & io_stream, std::string & o_id, std::string & o_name, std::string & o_version);

std::string platformNormalizeToken(const std::string & i_token);
bool platformDetectDetails(std::string & o_name_token, std::string & o_version_token);
std::vector<std::string> platformGetTokens();
}

