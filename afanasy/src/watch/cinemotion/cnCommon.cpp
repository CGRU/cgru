#include "../cinemotion/cnCommon.h"
#include <filesystem>

#include <sstream>
#include <chrono>
#include <cstdlib>
#include <cassert>

#include <iostream>

#ifdef WINNT
#include <Windows.h>
#endif
#ifdef LINUX
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

using namespace cn;

#pragma region Common

std::string Common::get_env(const std::string& var_name) {
	if (const char* env_j = std::getenv(var_name.c_str())) {
		return std::string(env_j);
	}
	return std::string();
}

std::string Common::get_current_path() {
	return std::filesystem::current_path().string();
}

std::string Common::get_executable_path() {
#if defined(WINNT)
	wchar_t path[FILENAME_MAX] = { 0 };
	GetModuleFileNameW(nullptr, path, FILENAME_MAX);
	return std::filesystem::path(path).parent_path().string();
#elif defined(LINUX)
	char path[FILENAME_MAX];
	ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
	return std::filesystem::path(std::string(path, (count > 0) ? count : 0)).parent_path().string();
#else
	std::assert(false, "Unexpected OS. Expect Windows or Linux");
#endif
}

std::string Common::get_parent_path(const std::string& path) {
	std::filesystem::path l_fpath = path;
	return l_fpath.parent_path().string();
}

std::string Common::join_path(const std::string& path, const std::string& append) {
	std::filesystem::path l_fpath  = path;
	std::filesystem::path l_append = append;
	return (l_fpath / l_append).string();
}

std::string Common::absolute_path(const std::string& path) {
	std::filesystem::path l_fpath = path;
	return std::filesystem::absolute(l_fpath).string();
}

bool Common::exists(const std::string& path) {
	std::filesystem::path l_fpath = path;
	return std::filesystem::exists(l_fpath);
}

bool Common::is_dir(const std::string& path) {
	std::filesystem::path l_fpath = path;
	return std::filesystem::is_directory(path);
}

bool Common::is_file(const std::string& path) {
	std::filesystem::path l_fpath = path;
	return std::filesystem::is_regular_file(path);
}

std::vector<std::string> Common::list_of_files(const std::string& dir) {
	std::vector<std::string> l_res;
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (std::filesystem::is_regular_file(entry.path()) == true) {
			l_res.push_back(entry.path().string());
		}
	}
	return l_res;
}

std::vector<std::string> Common::list_of_files(const std::string& dir, const std::vector<std::string>& filters) {
	std::vector<std::string> l_res;
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (std::filesystem::is_regular_file(entry.path()) == true) {
			std::string l_ext =  entry.path().extension().string();
			if (l_ext == ".py") {
				l_res.push_back(entry.path().string());
			}
		}
	}
	return l_res;
}

std::vector<std::string> Common::list_of_subdirs(const std::string& dir) {
	std::vector<std::string> l_res;
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (std::filesystem::is_directory(entry.path()) == true) {
			l_res.push_back(entry.path().string());
		}
	}
	return l_res;
}

std::string Common::file_name(const std::string& path, bool ext) {
	std::filesystem::path l_fpath = path;
	if (ext == true) {
		return l_fpath.filename().string();
	}
	return l_fpath.stem().string();

}

std::string Common::file_ext(const std::string& path) {
	std::filesystem::path l_fpath = path;
	return l_fpath.extension().string();
}

std::string Common::get_user_name() {
	return get_env("USER");    
}
std::vector<std::string> Common::split_string(const std::string& p_origin_str, const char split_symbol){
	std::stringstream l_ss(p_origin_str);
	std::string l_segment;
	std::vector<std::string> l_seg_list;

	while( std::getline(l_ss, l_segment, split_symbol) )
	{
		l_seg_list.push_back(l_segment);
	}
	return l_seg_list;
}
#pragma endregion
