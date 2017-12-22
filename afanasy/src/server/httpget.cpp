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
	httpget.cpp - get delivered simple webserver for static assets, like html, js and css files
*/

#include "httpget.hpp"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

std::vector<char *> HttpGet::http_get_blacklist_files = {
	"..",		// do not allow escaping the document root of the webserver
	"htdigest", // do not allow access to the .htdigest file
	"htaccess", // do not allow access to the .htaccess file
	".json"		// do not allow access to any json file
};

af::Msg *HttpGet::process(const af::Msg *i_msg)
{
	af::Msg *o_msg = new af::Msg();

	const std::string file_name = HttpGet::getFileNameFromInMsg(i_msg);
	const std::string mimeType = HttpGet::getMimeTypeFromFileName(file_name);

	// ### copy the file with proper http header to the output
	int file_size;
	char *file_data = NULL;
	if (file_name.size())
	{
		std::string error;
		file_data = af::fileRead(file_name, &file_size, -1, &error);
	}

	if (file_data)
	{
		std::string httpHeader = af::getHttpHeader(file_size, mimeType, "200 OK");

		// combine http header with file content into msg_data
		int msg_data_len = httpHeader.length() + file_size;
		char *msg_data = new char[msg_data_len];
		memcpy(msg_data, httpHeader.c_str(), httpHeader.length());
		memcpy(msg_data + httpHeader.length(), file_data, file_size);

		o_msg->setData(msg_data_len, msg_data, af::Msg::THTTPGET);

		delete[] file_data;
		delete[] msg_data;
	}
	else
	{
		std::string outputText404 = HttpGet::get404Content(file_name);
		std::string output404 = af::getHttpHeader(outputText404.length(), mimeType, "404 Not Found");
		output404 += outputText404;
		o_msg->setData(output404.size(), output404.c_str(), af::Msg::THTTPGET);
	}

	return o_msg;
}

std::string HttpGet::getFileNameFromInMsg(const af::Msg *i_msg)
{
	std::string file_name;

	static const char tasks_file[] = "@TMP@";
	static const int tasks_file_len = strlen(tasks_file);

	char *get = i_msg->data();
	int get_len = i_msg->dataLen();
	//::write( 1, get, get_len);
	int get_start = 4; // skipping "GET "
	int get_finish = get_start;
	while (get[++get_finish] != ' ')
		;
	while (get[get_start] == '/')
		get_start++;
	while (get[get_start] == '\\')
		get_start++;

	if (get_finish - get_start > 1)
	{
		file_name = std::string(get + get_start, get_finish - get_start);
		if (false == HttpGet::getValidateFileName(file_name))
		{
			AFCommon::QueueLogError("GET: Invalid file name from "
									+ i_msg->getAddress().v_generateInfoString() + "\n" + file_name);
			file_name.clear();
		}
		else if (file_name.find(tasks_file) == 0)
		{
			get_start += tasks_file_len;
			file_name = std::string(get + get_start, get_finish - get_start);
			if (file_name.find(af::Environment::getStoreFolder()) != 0)
			{
				AFCommon::QueueLogError("GET: Invalid @TMP@ folder from "
										+ i_msg->getAddress().v_generateInfoString() + "\n" + file_name);
				file_name.clear();
			}
			// printf("GET TMP FILE: %s\n", file_name.c_str());
		}
		else
		{
			file_name = af::Environment::getHTTPServeDir() + AFGENERAL::PATH_SEPARATOR + file_name;
		}
	}
	else
	{
		file_name = af::Environment::getHTTPServeDir() + AFGENERAL::HTML_BROWSER;
	}
	return file_name;
}

std::string HttpGet::getMimeTypeFromFileName(const std::string &filename)
{
	const std::string extension = filename.substr(filename.find_last_of(".") + 1);
	if (extension == "css") return "text/css";
	if (extension == "js") return "text/javascript";
	if (extension == "png") return "image/png";
	if (extension == "jpeg" || extension == "jpg") return "image/jpeg";
	if (extension == "gif") return "image/gif";

	return "text/html; charset=UTF-8";
}

bool HttpGet::getValidateFileName(const std::string &i_name)
{
	// do not serve files, which match an entry on the blacklist
	for (int i = 0; i < http_get_blacklist_files.size(); i++)
		if (i_name.find(http_get_blacklist_files[i]) != -1) return false;

	return true;
}

std::string HttpGet::get404Content(const std::string &filename)
{
	return std::string("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>AFANASY 404</title>")
		   + "<link type=\"text/css\" rel=\"stylesheet\" href=\"lib/styles.css\">"
		   + "<link type=\"text/css\" rel=\"stylesheet\" href=\"afanasy/browser/style.css\">"
		   + "</head><body id=\"afbody\" style=\"position: absolute; top: 50%;"
		   + "transform: translateY(-50%); text-align: center; width: 100%\">"
		   + "<span style=\"font-size: 30px;\">Arghh, page not found!</span><br>"
		   + "<span style=\"font-size: 100px; font-weight: bold;\">¯\\_(ツ)_/¯<br>404 Error</span><br>"
		   + "<span style=\"font-size: 20px;\">The requested file (" + filename
		   + ") could not be found on the server.</span><br>"
		   + "<span style=\"font-size: 15px;\">Contact the <a href=\"http://forum.cgru.info/\">forum</a>, "
		   + "if you think this is an error.</span></body></html>";
}
