#ifndef CN_DOC_TREE_H
#define CN_DOC_TREE_H

#include <string>
#include <istream>
#include <ostream>
#include <vector>
#include <memory>

namespace cn
{
	class DOCTree
	{
	public:
		DOCTree();
		bool save_to_json_file(const std::string& file_path);
		bool save_to_xml_file(const	std::string& file_path);
		bool save_to_ini_file(const std::string& file_path);
		bool save_to_info_file(const std::string& file_path);

		bool load_from_json_file(const std::string& file_path);
		bool load_from_xml_file(const std::string& file_path);
		bool load_from_ini_file(const std::string& file_path);
		bool load_from_info_file(const std::string& file_path);

		bool to_string_xml(std::string&);
		bool to_string_json(std::string&);
		bool from_string(const std::string&);

		bool put(const std::string& p_key, DOCTree p_tree_obj);
		bool put(const std::string& p_key, int p_value);
		bool put(const std::string& p_key, double p_value);
		bool put(const std::string& p_key, bool p_value);
		bool put(const std::string& p_key, const std::string& p_value);

		bool get(const std::string& p_key, DOCTree& p_tree_obj) const;
		bool get(const std::string& p_key, int& p_value) const;
		bool get(const std::string& p_key, double& p_value) const;
		bool get(const std::string& p_key, bool& p_value) const;
		bool get(const std::string& p_key, std::string& p_value) const;

		bool has_key(const std::string& l_str) const;
		std::vector<std::string> get_keys() const;
		int get_child_count() const;
		std::vector<std::string> get_string_array() const;

		//vufTreeObject get_child(const std::wstring& key);
		//void put_child(vufTreeObject*);

	private:
		std::shared_ptr<void> m_tree_ptr = nullptr;
	};
}

#endif