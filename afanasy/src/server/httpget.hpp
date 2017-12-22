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
	httpget.hpp - get delivered simple webserver for static assets, like html, js and css files
*/

#pragma once

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

class HttpGet
{
public:
	static af::Msg *process(const af::Msg *i_msg);

private:
	static std::vector<char *> http_get_blacklist_files;

	static std::string getFileNameFromInMsg(const af::Msg *i_msg);
	static bool getValidateFileName(const std::string &i_name);
	static std::string getHttpHeader(int file_size, const std::string &mimeType, const std::string &status);
	static std::string getMimeTypeFromFileName(const std::string &filename);
	static std::string get404Content(const std::string &filename);
};
