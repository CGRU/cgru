#pragma once

#include "af.h"

namespace af
{
class TaskData : public Af
{
  public:
	TaskData();

	TaskData(Msg *msg);

	/// Construct data from JSON:
	TaskData(const JSON &i_object);

	virtual ~TaskData();

	int calcWeight() const; ///< Calculate and return memory size.
	void v_generateInfoStream(std::ostringstream &o_str, bool full = false) const; /// Generate information.

	inline const std::string &getName() const { return m_name; }
	inline const std::string &getCommand() const { return m_command; }
	inline const std::string &getDependMask() const { return m_depend_mask; }
	inline const std::string &getCustomData() const { return m_custom_data; }
	inline const std::vector<std::string> &getFiles() const { return m_files; }
	inline const std::map<std::string, std::string> &getEnvironment() const { return m_environment; }

	inline bool hasFiles() const { return m_files.size(); }			   ///< Whether files are set.
	inline bool hasDependMask() const { return m_depend_mask.size(); } ///< Whether depend mask is set.
	inline bool hasCustomData() const { return m_custom_data.size(); } ///< Whether files are set.
	inline bool hasEnvironment() const { return m_environment.size(); }
	bool checkDependMask(const std::string &str);

	void jsonRead(const JSON &i_object);
	void jsonWrite(std::ostringstream &o_str) const;

  protected:
	std::string m_name;								  ///< Task name.
	std::string m_command;							  ///< Command.
	std::string m_depend_mask;						  ///< Dependences.
	std::string m_custom_data;						  ///< Some custom data.
	std::vector<std::string> m_files;				  ///< Files.
	std::map<std::string, std::string> m_environment; ///< Task extra environment.

  protected:
	/// Read or write task data.
	virtual void v_readwrite(Msg *msg);
};
} // namespace af
