#ifndef CN_STRING_PARSER_H
#define CN_STRING_PARSER_H

#include <string>
#include <sstream>
#include <vector>

namespace cn {
	class StringParser
	{
	public:


		static bool parse_to_vectors(	std::vector<std::string>& parsed_vec,
										std::vector<std::string>& arg_vec,										
										const char* str) 
		{			
			std::string l_str(str);
			parsed_vec.clear();
			parsed_vec.clear();
			
			std::string l_parse_tmp;
			while (true) {
				// find { check special case \{
				size_t found_open = 0;
				while (true) {
					found_open = l_str.find('{');
					if (found_open == std::string::npos) {
						// There isn't any open bracket	
						//std::cout << "{ npos" << std::endl;
						parsed_vec.push_back(l_str);
						return true;
					}
					if (found_open == 0) {
						//{ is in the start of string
						//std::cout << "{ in the start" << std::endl;
						l_str = l_str.substr( 1,l_str.length());
						l_parse_tmp = "";
						if (parsed_vec.empty() ==true) parsed_vec.push_back("");
 						break;
					}
					/*
					if (l_str[found_open - 1] == '\\') {
						// in case '\{'
						l_parse_tmp = l_parse_tmp + l_str.substr(0, found_open );
						l_str = l_str.substr(found_open, l_str.length() - found_open);
						
						found_open = 0;
						std::cout << "\\{" << std::endl;
						continue;
					}
					*/
					// others cases
					//std::cout << "__str " << l_str.substr(0, found_open) << std::endl;
					//break;
					parsed_vec.push_back(l_parse_tmp + l_str.substr(0, found_open));
					l_str = l_str.substr(found_open + 1, l_str.length() - found_open -1);
					l_parse_tmp = "";

					//std::cout << "{ str " << l_str << std::endl;
					//std::cout << "{ common " << std::endl;
					break;
				}
				// To Do the same to found close				
				//std::cout << "}" << std::endl;
				size_t found_close = l_str.find('}');
				if (found_close == std::string::npos) {
					return false;
				}
				std::string l_cut_before = l_str.substr(0, found_close);
				arg_vec.push_back(l_cut_before);
				l_str = l_str.substr(found_close +1,l_str.length() - found_close );
				//std::cout << "} l_str " << l_str << std::endl;
				
			}
			return true;
		}


		template<typename... Args>
		static std::string parse(const char* str, Args... args)
		{
			std::stringstream l_ss;
			std::vector<std::string> l_vec;
			to_vec_h(l_vec, args...);
			
			std::string l_str(str);
			//std::cout << l_vec.size() << std::endl;
			//for (auto& i : l_vec) std::cout << i << " ";
			//std::cout << std::endl;

			int ndx;
			while (true) {
				// find { check special case \{
				size_t found_open = 0;
				while (true) {
					found_open	= l_str.find('{');
					if (found_open == std::string::npos) {
						l_ss << l_str;
						return l_ss.str();
					}
					if (found_open == 0) break;
					if (l_str[found_open - 1] == '\\') {
						l_ss << l_str.substr(0, found_open -1 ) << '{';
						//std::cout << " get " << l_str.substr(0, found_open + 1) << std::endl;
						l_str = l_str.substr(found_open + 1, l_str.length()- found_open);
						found_open = 0;
						continue;
					}
					break;
				}

				// To Do the same to found close

				size_t found_close	= l_str.find('}',found_open);
				if (found_close == std::string::npos || found_close < found_open) {
					l_ss << l_str;
					break;
				}
				std::string l_cut_before = l_str.substr(0, found_open);
				l_ss << l_cut_before;
				std::stringstream l_ss_2;
				
				l_ss_2 << l_str.substr(found_open + 1, found_close - found_open -1);
				//std::cout << " index " << l_str.substr(found_close + 1, found_close - found_open - 1) << std::endl;
				l_ss_2 >> ndx;
				if (ndx < l_vec.size()) {
					l_ss << l_vec[ndx];
				}
				else
				{
					l_ss << "##wrng ndx##";
				}
				
				l_str = l_str.substr(found_close + 1, l_str.length() - found_close);
				//std::cout << l_str << std::endl;
			}
			return l_ss.str();
		}
		
		template<typename T>
		static std::string parse(const T& val)
		{
			std::stringstream l_ss;
			l_ss << val;
			return l_ss.str();
		}
		// 
		static std::string parse(const char* str)
		{
			return std::string(str);
		}

	private:
		// convert data to string array
		template<typename T, typename... Args>
		static void to_vec_h( std::vector<std::string>& vec,const T& val,Args... args )
		{
			std::stringstream l_ss;
			l_ss << std::noshowpoint << val;
			vec.push_back(l_ss.str());
			to_vec_h(vec, args...);
		}
		
		// convert data to string array
		template<typename T>
		static void to_vec_h(std::vector<std::string>& vec, const T& val)
		{
			std::stringstream l_ss;
			l_ss << std::noshowpoint << val;
			vec.push_back(l_ss.str());
		}
	};
}

#endif //!CN_STRING_PARSER_H