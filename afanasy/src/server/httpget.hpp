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
