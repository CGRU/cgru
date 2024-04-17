#ifndef CD_COMON_H
#define CD_COMON_H

#include <string>
#include <vector>
#include <chrono>

namespace cn
{
	class Common {
		public:
		static std::string 				get_env(const std::string& var_name);
		static std::string 				get_current_path();
		static std::string 				get_executable_path();
		static std::string 				get_parent_path(const std::string&);
		static std::string 				join_path(const std::string&, const std::string&);
		static std::string 				absolute_path(const std::string&);
		static bool 					exists(const std::string&);
		static bool 					is_dir(const std::string&);
		static bool 					is_file(const std::string&);
		static std::vector<std::string> list_of_files(const std::string& dir);
		static std::vector<std::string> list_of_files(const std::string& dir, const std::vector<std::string>& filters);
		static std::vector<std::string> list_of_subdirs(const std::string& dir);
		static std::string 				file_name(const std::string&, bool ext = true);
		static std::string 				file_ext(const std::string&);

		static std::string 				get_user_name();
		
		static std::vector<std::string> split_string(const std::string& origin, const char split_symbol);
	};
}

#endif