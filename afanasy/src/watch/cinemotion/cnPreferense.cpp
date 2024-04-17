#include "../cinemotion/cnPreferense.h"
#include "../cinemotion/cnCommon.h"
#include "../cinemotion/cnDOCTree.h"
#include "../cinemotion/cnLoger.h"
#include "../cinemotion/cnStdLoger.h"
#include "../cinemotion/cnInclude.h"
#include <vector>
#include <algorithm>

using namespace cn;

const MenuVec cn::g_menu_set_null;

#pragma region MENU
std::shared_ptr<Menu> Menu::create() {
	return std::shared_ptr<Menu>(new Menu());
}

std::string Menu::get_name() const { 
	return m_name;
}
void Menu::set_name(const std::string& name) {
	m_name = name;
}

std::string Menu::get_cmd() const {
	return m_cmd;
}
void Menu::set_cmd(const std::string& cmd) {
	m_cmd = cmd;
}

std::string Menu::get_arg() const {
	return m_arg;
}
void Menu::set_arg(const std::string& arg ) {
	m_arg = arg;
}

bool Menu::is_enable_when_done() const {
	return m_enable_when_done;
}
void Menu::set_enable_when_done(bool val) {
	m_enable_when_done = val;
}

bool Menu::is_multi() const {
	return m_multi;
}
void Menu::set_multi(bool val) {
	m_multi = val;
}

std::shared_ptr<PopupMenu> Menu::get_popup_menu() const{
	if (m_stage_ptr.lock() == nullptr)
		return nullptr;
	return m_stage_ptr.lock()->get_popup_menu();
}
std::shared_ptr<Stage> Menu::get_stage() const {
	 return m_stage_ptr.lock();
}

void Menu::add_service(const std::string& service) {
	m_services.insert(service);
}
bool Menu::has_service(const std::string& service) const {
	return m_services.find(service) != m_services.end();
}
bool Menu::support_service(const std::string& service) const {
	if (m_services.empty() == true)
		return true;
	return has_service(service);
}
std::vector<std::string> Menu::extract_unsupport_services(const std::set<std::string>& candidates) const{
	std::vector<std::string> l_res;
	if (m_services.empty() == true){
		return l_res;
	}
	
	std::set<std::string> excluded;

	for (const auto& candidate: candidates) {
		bool l_exist = false;
		for (const auto& my_item: m_services) {
			if (candidate == my_item ) {
				l_exist = true;
			}
		}
		if (l_exist == false) {
			excluded.insert(candidate);
		}
	}

	for (const auto& item: excluded) {
		l_res.push_back(item);
	}
	return l_res;
}

bool Menu::get_enable_for( 	bool p_multi_selected, 
							bool all_done,
							const std::set<std::string>& p_services,
							std::string& ext_menu_name ) {
	bool l_enable = true;
	ext_menu_name = m_name;
	// check multi selection
	if ( p_multi_selected == true && is_multi() == false ) {

		ext_menu_name += " (Single)";
		l_enable = false;
	}
	// check enable when done
	if ( all_done == false && is_enable_when_done() == true ) {

		ext_menu_name += " (When All Done)";
		l_enable = false;
	}

	// 
	std::vector<std::string> l_res =  extract_unsupport_services(p_services);
	if (l_res.empty() == false ) {
		ext_menu_name += " (Not My Service:[ ";
		for (const auto& serv : l_res) {
			ext_menu_name += serv + " ";
		}
		ext_menu_name += "])";
		l_enable = false;
	}
	return l_enable;
}
#pragma endregion


#pragma region STAGE
std::shared_ptr<Stage> Stage::create() {
	return std::shared_ptr<Stage>(new Stage());
}
std::string Stage::get_name() const {
	return m_name;
}

void Stage::set_name(const std::string& name) {
	m_name = name;
}

void Stage::add_menu(std::shared_ptr<Menu> l_menu) {
	l_menu->m_stage_ptr = shared_from_this();
	m_menus.push_back(l_menu);
}

bool Stage::empty() const{
	return m_menus.empty();
}
#pragma endregion


#pragma region POPUP_MENU

std::shared_ptr<PopupMenu> PopupMenu::create() {
	return std::shared_ptr<PopupMenu>(new PopupMenu);
}

std::string PopupMenu::get_name() const { 
	return m_name; 
}

void PopupMenu::set_name(const std::string& name){
	m_name = name;
}

std::string PopupMenu::get_source() const{
	return m_source_path;
}

void PopupMenu::set_source(const std::string& src){
	m_source_path = src;
}

void PopupMenu::add_env(const std::string& p_var, const std::string& p_val)
{
	m_env[p_var]  = p_val;
	m_dict[p_var] = p_val;
}

void PopupMenu::add_path(const std::string& p_var, const std::string& p_val){
	m_paths[p_var] = p_val;
	m_dict[p_var] = p_val;
}
		
const MenuVec& PopupMenu::get_menus(const std::string& p_stage_name) const{

	if (m_stages.find(p_stage_name) == m_stages.end()){
		return g_menu_set_null;
	}		
	return m_stages.at(p_stage_name)->get_menus();
}

const StageMap& PopupMenu::get_stages() const {
	return m_stages;
}

bool PopupMenu::has_stage(const std::string& p_stage_name) const {
	if ( m_stages.find(p_stage_name) != m_stages.end() ) {
		return true;
	}
	return false;
}

void PopupMenu::add_stage(std::shared_ptr<Stage> stage) {
	stage->m_popup_ptr = shared_from_this();
	m_stages[stage->get_name()] = stage;
}

bool PopupMenu::empty() const {
	return m_stages.empty();
}

bool PopupMenu::has_dict_key(const std::string& key) const {
	return m_dict.find(key) !=m_dict.end();

}
std::string PopupMenu::get_dict_val(const std::string& key) const {
	if ( m_dict.find(key) !=m_dict.end())
		return m_dict.at(key);
	return "NAN";
}
void PopupMenu::add_dict_val(const std::string& key, const std::string& val) {
	m_dict[key] = val;
}

bool PopupMenu::generate_cmd(	const std::vector<std::string>& base, 
								const std::vector<std::string>& arg,
								std::string& res)
{
	for (int j = 0; j < (int)base.size(); ++j)
	{								
		//l_menu.add
		res += base[j];
		if (j < arg.size())
			res += get_dict_val(arg[j]); 
	}
	return true;
}

void PopupMenu::reset_env_nan() {
	//auto it = m_env.find("JOB_ID");
	m_env.erase("JOB_ID");
	m_env.erase("BLOCK_ID");
	m_env.erase("TASK_ID");
}
#pragma endregion


#pragma region PREF

std::shared_ptr<Pref>  Pref::g_ptr = nullptr;

std::shared_ptr<Pref>  Pref::get() {
	if (g_ptr == nullptr)
	{
		g_ptr = std::shared_ptr<Pref>(new Pref());
		g_ptr->init();
	}
	return g_ptr;
}

bool Pref::has_pop_up_name(const std::string& name) const {
	return ( m_popups.find(name) != m_popups.end() );
}

// read menus
bool Pref::init(){
	m_cgru_path = Common::get_env("CGRU_LOCATION");	
	std::string l_exe_path = Common::get_executable_path();
	
	CN_RISE();
	CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_BLUE, "Start to read prefs");
	CN_DIVE();

	// Read from CGRU_LOCATION
	if (m_cgru_path.empty() == false && Common::is_dir(m_cgru_path) == true)
	{		
		std::string l_json_dir = Common::join_path(m_cgru_path, m_search_folder);
		CN_INFO(l_json_dir);
		CN_DIVE()
		read_from_json(l_json_dir);	
		CN_UP();	
	}
	// Read from executable path
	{
		std::string l_json_dir = Common::join_path(l_exe_path, m_search_folder);
		CN_INFO(l_json_dir);
		CN_DIVE();
		read_from_json(l_json_dir);	
		CN_UP()
	}
	// Read from home path
	{
		std::string l_path_1= Common::join_path("/home",Common::get_user_name());
		std::string l_json_dir = Common::join_path(l_path_1, m_search_folder);
		CN_INFO(l_json_dir);
		CN_DIVE();
		read_from_json(l_json_dir);	
		CN_UP()
	}
	CN_UP();
	CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_BLUE," End Read Environs...");

	CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_AQUA," ----------------");
	log();

	return true;
}

bool Pref::read_from_json(const std::string& p_json_dir) {

	if (Common::is_dir(p_json_dir) == true) {
		CN_INFO("Read prefs from ",p_json_dir );
		// Get files in directory
		auto l_files = Common::list_of_files(p_json_dir);
		CN_DIVE()	
		for (auto& l_file: l_files ) {
			if (Common::file_ext(l_file) != ".json") {
				continue;
			}
			CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_BLUE, "Start Parse Pref File ",l_file );

			// Do Read
			DOCTree		l_tree;
			if (l_tree.load_from_json_file(l_file) == true) {
				std::string l_name = Common::file_name(l_file, false);
				if (has_pop_up_name(l_name) == true) {
					CN_WARN( "Pop Up Menu ",l_name, " already exists." );
					CN_UP();
					return false;
				}
				auto l_pop_up = PopupMenu::create();
				l_pop_up->set_name(l_name);
				l_pop_up->set_source(l_file);

				// read enverons
				if (l_tree.has_key("environs")) {
					
					DOCTree l_environ_tree;
					l_tree.get("environs", l_environ_tree);
					std::vector<std::string> l_environ_keyes = l_environ_tree.get_keys();
					for (auto& key:l_environ_keyes){
						std::string env_value;
						// To Do
						//try block 	
						if (l_environ_tree.get(key,env_value) == false) {
							env_value = Common::get_env(key);
						}
						if (env_value.empty() == true) env_value = key;
						//CN_INFO()
						l_pop_up->add_env(key, env_value);
						CN_INFO("[ENV] ", key, ": ",env_value );
					}
				}
				
				// read paths
				if (l_tree.has_key("paths") == true) {
					DOCTree l_path_tree;
					l_tree.get("paths", l_path_tree);
					std::vector<std::string> l_path_keyes = l_path_tree.get_keys();

					for (auto& key:l_path_keyes ) {
						std::string l_path;
						//CN_INFO("[PATH] ", key, ": ",l_path );							
						if ( l_path_tree.get(key, l_path) == true && l_path.empty() == false ) {
							if (cn::Common::is_dir(l_path) == true)	{
								l_pop_up->add_path(key, l_path);
								CN_INFO("[PATH] ", key, ": ",l_path );
							}
							else{
								CN_WARN("[PATH] Doesnt exist. Skipped ", key, ": ",l_path );
							}
						}
					}
				}			

				// read actions
				if (l_tree.has_key("actions")){
					DOCTree l_actions_tree;
					l_tree.get("actions", l_actions_tree);
					std::vector<std::string> l_action_keyes = l_actions_tree.get_keys();
					
					// read stages
					for (auto& stage_key: l_action_keyes ){
						if (stage_key == "job" || stage_key == "block" || stage_key == "task"){
							DOCTree l_stage_tree;
							l_actions_tree.get(stage_key, l_stage_tree);							
							auto l_stage = Stage::create();
							l_stage->set_name(stage_key);

							// read menus
							std::vector<std::string> l_menu_keyes = l_stage_tree.get_keys();
							for (auto& menu_key: l_menu_keyes ) {
								DOCTree l_menu_tree;
								l_stage_tree.get(menu_key, l_menu_tree);
								auto l_menu = Menu::create();
								l_menu->set_name(menu_key);								
								
								// cmd option
								if (l_menu_tree.has_key("cmd") == false) {
									CN_WARN("Can't find cmd for ",menu_key, " Skipped.");
									continue;
								}
								else{
									std::string l_val;
									l_menu_tree.get("cmd",l_val);
									l_menu->set_cmd(l_val);
								}
								 
								// arg option
								if (l_menu_tree.has_key("arg") == true) {
									std::string l_val;
									l_menu_tree.get("arg",l_val);
									l_menu->set_arg(l_val);
								}
								
								// enable when done option
								if (l_menu_tree.has_key("when_done") == true) {
									l_menu->set_enable_when_done(true);
								}

								// does work for multi selected work items								
								if (l_menu_tree.has_key("multi") == true) {
									l_menu->set_multi(true);
								}

								if (l_menu_tree.has_key("service") == true) {
									DOCTree l_service_tree;
									l_menu_tree.get("service", l_service_tree);
									auto val_arr = l_service_tree.get_string_array();
									for (auto serv: val_arr ){
										l_menu->add_service(serv);
									}
								}

								l_stage->add_menu(l_menu);
								CN_INFO("STAGE ",l_stage->get_name(), " MENU: ", l_menu->get_name());
							}
							
							CN_WARN("STAGE_EMPTY: ",l_pop_up->empty());
							if (l_stage->empty() == false )
								l_pop_up->add_stage(l_stage);
						}
					}

				}
				
				// add pop up
				CN_WARN("POP_UP_EMPTY: ",l_pop_up->empty());
				if (l_pop_up->empty() == false){
					l_pop_up->add_dict_val("CGRU_LOCATION",m_cgru_path);
					m_popups[l_pop_up->get_name()] = l_pop_up;
				}
			}
			CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_BLUE, "End Parse Pref File ",l_file);
		}
		CN_UP();
	}

	return true;
}

void Pref::log() const {
	CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_BLUE,"========Start Pref Log========");
	CN_DIVE();

	for (const auto& pop_up: m_popups) {
		CN_INFO("Pop up menu ",  pop_up.first);
		CN_INFO("Source: File: ",pop_up.second->get_source());
		const auto& stages = pop_up.second->get_stages();
		for (const auto& stage: stages) {
			CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_BLUE,"Stage: [",stage.second->get_name(),"]" );
			CN_DIVE();
			const auto& menus = stage.second->get_menus();
			for (const auto& menu: menus) {
				CN_INFO("[MENU] ",menu->get_name());
				CN_DIVE();
				CN_INFO("cmd ",    menu->get_cmd());
				CN_INFO("arg ",    menu->get_arg());
				CN_INFO("multi ",  menu->is_multi());
				CN_INFO("en_done ",menu->is_enable_when_done());
				CN_UP();
			}
			CN_UP();
		}
	}

	CN_UP();
	CN_INFO_COL(CN_CONSOLE_COLOR_LIGHT_BLUE,"=========End Pref Log=========");
}
#pragma endregion
