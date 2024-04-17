#include "../cinemotion/cnDOCTree.h"
#include "../cinemotion/cnLoger.h"
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

using namespace cn;
namespace pt = boost::property_tree;

DOCTree::DOCTree()
{
	m_tree_ptr = std::shared_ptr<pt::ptree>(new pt::ptree());
}
bool DOCTree::save_to_json_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to save json file ", p_file, " .Property tree is absent. ");
		return false;
	}

	try
	{
		pt::write_json(p_file, *l_pt.get(), std::locale(""));
		//pt::json_parser::write_json(l_path,*l_pt.get(),std::locale(""));
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to save json file ", p_file, " ", p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::save_to_xml_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there isnt ptree object
		CN_ERR("Failed to save xml file. ", p_file, "Property tree is absent.");
		return false;
	}

	try
	{
		//boost::property_tree::xml_writer_settings<wchar_t> l_settings(L'\t', 1);
		pt::xml_writer_settings<std::string> l_w(' ', 3);
		pt::write_xml(p_file, *l_pt.get(), std::locale(""), l_w);
		//pt::write_xml(l_path, *l_pt.get(), std::locale(""), l_w);
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to save xml file ", p_file, " ", p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::save_to_ini_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to save ini file ", p_file, " .Property tree is absent.");
		return false;
	}

	try
	{
		pt::write_ini(p_file, *l_pt.get(), 0, std::locale(""));
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to save ini file ", p_file, " ", p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::save_to_info_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to save info file. ", p_file, " .Property tree is absent.");
		return false;
	}

	try
	{
		pt::write_info(p_file, *l_pt.get(), std::locale(""));// , std::locale());
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to save info file ", p_file, " ", p_e.what());
		return false;
	}
	return true;
}

bool DOCTree::load_from_json_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to load from json file ", p_file, " .Property tree is absent.");
		return false;
	}

	try
	{
		pt::read_json(p_file, *l_pt.get(), std::locale(""));
		//pt::json_parser::write_json(l_path,*l_pt.get(),std::locale(""));
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to load from json file ", p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::load_from_xml_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed ot load from xml file ", p_file, " .Property tree is absent.");
		return false;
	}

	try
	{
		pt::read_xml(p_file, *l_pt.get(), 0, std::locale(""));
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to load from xml_file ", p_file, p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::load_from_ini_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to load from ini file ", p_file, " Property tree is absent.");
		return false;
	}
	try
	{
		pt::read_ini(p_file, *l_pt.get(), std::locale(""));
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to load file ", p_file, " ", p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::load_from_info_file(const std::string& p_file)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to load from info file ", p_file, " .Property tree is absent.");
		return false;
	}
	try
	{
		pt::read_info(p_file, *l_pt.get(), std::locale(""));
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to load from info file ", p_file, p_e.what());
		return false;
	}
	return true;
}

bool DOCTree::to_string_xml(std::string& p_str)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("to_string xml  is failed. Property tree is absent.");
		return false;
	}

	std::stringstream l_ss;
	try
	{
		pt::xml_writer_settings<std::string> l_w(L' ', 3);
		pt::write_xml(l_ss, *l_pt.get(), l_w);
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to string xml ", p_e.what());
		return false;
	}
	p_str = l_ss.str();
	return true;
}
bool DOCTree::to_string_json(std::string& p_str)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("to_string json is failed. Property tree is absent.");
		return false;
	}

	std::stringstream l_ss;
	try
	{
		pt::write_json(l_ss, *l_pt.get());
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to string json ", p_e.what());
		return false;
	}
	p_str = l_ss.str();
	return true;
}
bool DOCTree::from_string(const std::string& p_str)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("from_string is failed. Property tree is absent.");
		return false;
	}
	std::stringstream l_ss;
	l_ss << p_str;
	try
	{
		pt::read_xml(l_ss, *l_pt.get());
	}
	catch (std::exception& p_e)
	{
		CN_ERR("Failed to string ", p_e.what());
		return false;
	}
	return true;
}

bool DOCTree::put(const std::string& p_key, DOCTree p_tree_obj)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	std::shared_ptr<pt::ptree> l_pt_target = std::static_pointer_cast<pt::ptree>(p_tree_obj.m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Put object is invalid. Property tree is null.");
		return false;
	}
	if (l_pt_target == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Put object is Invalid. Target property tree is null.");
		return false;
	}

	try
	{
		l_pt->add_child(p_key, *l_pt_target.get());
	}
	catch (const std::exception& l_e)
	{
		CN_ERR("Failed to put object into key ", p_key, " ", l_e.what());
		return false;
	}
	return true;
}
bool DOCTree::put(const std::string& p_key, int p_value)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		CN_ERR("Failed to put int into key ", p_key, " .Property tree is null.");
		return false;
	}
	try
	{
		l_pt->put(p_key, p_value);
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to put int to key ", p_key, " ", p_e.what());
		return false;
	}

	return true;
}
bool DOCTree::put(const std::string& p_key, double p_value)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to put double into key ", p_key, " .Property tree is null.");
		return false;
	}
	try
	{
		l_pt->put(p_key, p_value);
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to put double into key ", p_key, " ", p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::put(const std::string& p_key, bool p_value)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed tp put bool into key ", p_key, " ", " .Property tree is null.");
		return false;
	}
	try
	{
		l_pt->put(p_key, p_value);
	}
	catch (const std::exception& p_e)
	{
		CN_ERR("Failed to put bool into key ", p_key, " ", p_e.what());
		return false;
	}
	return true;
}
bool DOCTree::put(const std::string& p_key, const std::string& p_value)
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to put string into key ", p_key, " ", " .Property tree is null.");
		return false;
	}
	try
	{
		l_pt->put(p_key, p_value);
	}
	catch (const std::exception& p_e)
	{

		CN_ERR("Failed to put string into key ", p_key, " ", p_e.what());
		return false;
	}
	return true;
}

bool DOCTree::get(const std::string& p_key, DOCTree& p_tree_obj) const
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to get ptree form key ", p_key, " .Property tree is null.");
		return false;
	}
	pt::ptree l_ptree;
	try
	{
		l_ptree = l_pt->get_child(p_key);
	}
	catch (const std::exception& l_e)
	{
		CN_ERR("Failed to get ptree from key ", p_key, " ", l_e.what());
		return false;
	}
	std::shared_ptr<pt::ptree> l_tree_ptr = std::shared_ptr<pt::ptree>(new pt::ptree(l_ptree));
	p_tree_obj.m_tree_ptr = l_tree_ptr;
	return true;
}
bool DOCTree::get(const std::string& p_key, int& p_value) const
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to get int from key ", p_key, " .Property tree is null.");
		return false;
	}
	int l_val;
	try
	{
		l_val = l_pt->get<int>(p_key);
	}
	catch (const std::exception& l_e)
	{
		CN_ERR("Failed to get int form key ", p_key, " ", l_e.what());
		return false;
	}
	p_value = l_val;
	return true;
}
bool DOCTree::get(const std::string& p_key, double& p_value) const
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to get double form key ", p_key, " .Property tree is null.");
		return false;
	}
	double l_val;
	try
	{
		l_val = l_pt->get<double>(p_key);
	}
	catch (const std::exception& l_e)
	{
		CN_ERR("Failed to get double form key ", p_key, " ", l_e.what());
		return false;
	}
	p_value = l_val;
	return true;
}
bool DOCTree::get(const std::string& p_key, bool& p_value) const
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to get bool from key ", p_key, " .Property tree is null.");
		return false;
	}
	bool l_val;
	try
	{
		l_val = l_pt->get<bool>(p_key);
	}
	catch (const std::exception& l_e)
	{
		CN_ERR("Failed to get bool from key ", p_key, " ", l_e.what());
		return false;
	}
	p_value = l_val;
	return true;
}
bool DOCTree::get(const std::string& p_key, std::string& p_value) const
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr)
	{
		// there is'nt ptree object
		CN_ERR("Failed to get string from key ", p_key, " .Property tree is null.");
		return false;
	}
	std::string l_val;
	try
	{
		l_val = l_pt->get<std::string>(p_key);
	}
	catch (const std::exception& l_e)
	{
		CN_ERR("Failed to get string from key ", p_key, " ", l_e.what());
		return false;
	}
	p_value = l_val;
	return true;
}
bool DOCTree::has_key(const std::string& l_str) const{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr) return false;
	for (const auto& child : *l_pt.get()) {
		if ( child.first ==l_str) return true;
	}
	return false;
}
std::vector<std::string> DOCTree::get_keys() const
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	std::vector < std::string> l_keys;
	if (l_pt == nullptr) return l_keys;
	for (auto& child : *l_pt.get())
	{
		l_keys.push_back(child.first);
	}
	return l_keys;
}
int DOCTree::get_child_count() const
{
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	if (l_pt == nullptr) return -1;
	return (int)l_pt->size();
}
std::vector<std::string> DOCTree::get_string_array() const {
	std::shared_ptr<pt::ptree> l_pt = std::static_pointer_cast<pt::ptree>(m_tree_ptr);
	std::vector<std::string> l_vec;
	if (l_pt == nullptr) return l_vec;
	for (auto& child : *l_pt.get()) {
		l_vec.push_back(child.second.get_value<std::string>());
	}
	return l_vec;
}
