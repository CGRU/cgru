#ifndef CN_PREF_H
#define CN_PREF_H

#include <map>
#include <set>
#include <memory>
#include <vector>
#include "../cinemotion/cnDOCTree.h"
#include <functional>
#include <memory>

namespace cn {

	class ItemBase {
	public:
		int64_t get_id() const {return m_id;}
		void 	set_id(int64_t new_id)  {m_id = new_id; }

		std::string get_service() const {return m_service;}
		void 		set_service (const std::string& serv) {m_service = serv;}

		bool 	get_done() const { return m_done; }
		void 	set_done(bool dn) { m_done = dn; }

	private:
		int64_t 	m_id = 0;
		std::string m_service;
		bool 		m_done;
	};

	struct TaskInfo {
		int64_t 	m_id = 0;
		int64_t		m_job_id = 0;
		int64_t		m_block_id = 0;
		std::string m_service;
		bool 		m_done;
	};

	struct BlockInfo {
		int64_t		m_id = 0;
		int64_t		m_job_id = 0;
		std::string m_service;
		bool 		m_done;
	};

	struct JobInfo {
		int64_t 	m_id = 0;
		std::string m_service;
		bool 		m_done;		
	};
	
	

	class Pref;
	class PopupMenu;
	class Stage;
	class Menu;
	class MenuComparator;

	using MenuVec  = std::vector< std::shared_ptr<Menu> >;//,MenuComparator>;
	using StageMap = std::map<std::string, std::shared_ptr<Stage>>;
	using PopUpMap = std::map<std::string, std::shared_ptr<PopupMenu>>;
	extern const MenuVec g_menu_set_null;

	

	class  Menu: public std::enable_shared_from_this<Menu>
	{
		private:
			Menu() = default;
		public:
			static std::shared_ptr<Menu> create();
		public:
			std::string get_name() const;
			void set_name(const std::string&);

			std::string get_cmd() const;
			void set_cmd(const std::string& );

			std::string get_arg() const;
			void set_arg(const std::string& );

			bool is_enable_when_done() const;
			void set_enable_when_done(bool);

			bool is_multi() const;
			void set_multi(bool);
			 
			std::shared_ptr<PopupMenu> get_popup_menu() const;			
			std::shared_ptr<Stage> get_stage() const;

			void add_service(const std::string& service);
			bool has_service(const std::string& service) const;
			bool support_service(const std::string& service) const;
			std::vector<std::string> extract_unsupport_services(const std::set<std::string>&) const;
								

			bool get_enable_for(bool m_multi_selected, 
								bool all_done,
								const std::set<std::string>& p_service,
								std::string& ext_menu_name);
		private:
			std::string m_name;
			std::string m_cmd;
			std::string m_arg;
			bool m_enable_when_done = false;
			bool m_multi = false;
			std::set<std::string> m_services;
		public:
			std::weak_ptr<Stage> m_stage_ptr;
	};

	class MenuComparator{
	public:
		bool operator() (const Menu& m_1, const Menu& m_2) const{
			std::hash<std::string> l_1;
			std::hash<std::string> l_2;
			return l_1(m_1.get_name())< l_2(m_2.get_name());
		}
	};

	class Stage: public std::enable_shared_from_this<Stage>
	{
		private:
			Stage() = default;
		public:
			static std::shared_ptr<Stage> create();
			std::string get_name() const;
			void set_name(const std::string&);

			bool empty() const;
			void add_menu(std::shared_ptr<Menu> l_menu);

			std::shared_ptr<PopupMenu> get_popup_menu() { return m_popup_ptr.lock(); }
		public:
			const MenuVec& get_menus() const {return m_menus; }
		private:
			std::string m_name;
			MenuVec m_menus;
		public:
			std::weak_ptr<PopupMenu> m_popup_ptr;

		
		//PopupMenu* m_parent = nullptr;
	};

	class PopupMenu: public std::enable_shared_from_this<PopupMenu>
	{
		private:
			PopupMenu() = default;
		public:
			static std::shared_ptr<PopupMenu> create();
			std::string 	get_name() const;
			void 			set_name(const std::string&);

			std::string 	get_source() const;
			void 			set_source(const std::string&);

			void 			add_env(const std::string&, const std::string&);
			void 			add_path(const std::string&, const std::string&);

			const MenuVec& 	get_menus(const std::string& p_stage_name) const;
			const StageMap& get_stages() const;

			bool has_stage(const std::string& p_stage_name) const;
			void add_stage(std::shared_ptr<Stage> );

			bool has_dict_key(const std::string&) const;
			void add_dict_val(const std::string& key, const std::string& val);
			std::string get_dict_val(const std::string&) const;

			bool empty() const;

			bool generate_cmd(	const std::vector<std::string>& base, 
								const std::vector<std::string>& arg,
								std::string& res);
			void log() const;

			void reset_env_nan();
		private:
			std::map<std::string, std::string> 	m_env;
			std::map<std::string, std::string> 	m_paths;
			std::map<std::string, std::string> 	m_dict;

			std::string m_source_path;
			std::string m_name;

			StageMap m_stages;
	};

	// usings
	// menu/stage/action
	class Pref{
	private:
		Pref(){};
	public:
		static std::shared_ptr<Pref> get();
		const PopUpMap&   get_popups() const { return m_popups;}
		void              log() const;
		bool              has_pop_up_name(const std::string&) const;
	
	private:
		bool init();
		bool init(const std::string& p_menu, const std::string& m_file_path);

		bool read_from_json(const std::string&);		

		static std::shared_ptr<Pref> g_ptr;
	private: 
		const std::string m_search_folder 	= "afwatch_config";

		std::string m_cgru_path;

		PopUpMap m_popups;
	};
}


#endif