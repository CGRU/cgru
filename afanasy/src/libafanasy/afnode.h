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
	afnode.h - Afanasy node.
	af::Node is a base class for any node that server stores in containers.
*/

#pragma once

#include <algorithm>
#include <list>

#include "af.h"

class AfContainer;
class AfContainerIt;
class AfList;
class AfListIt;
class AfNodeSrv;
class MonitorContainer;
class RenderAf;

namespace af
{
/// Afanasy Node.
class Node : public Af
{
  public:
	Node();
	virtual ~Node();

	inline int getId() const { return m_id; }					  ///< Get Node id.
	inline int getPriority() const { return m_priority; }		  ///< Get Node priority.
	inline void getName(std::string &str) const { str = m_name; } ///< Get Node name.
	inline const std::string &getName() const { return m_name; }  ///< Get Node name.

	inline int64_t getFlags() const { return m_flags; }
	inline int64_t getState() const { return m_state; }

	inline bool hasCustomData() const { return m_custom_data.size() != 0; }
	inline bool noCustomData() const { return m_custom_data.size() == 0; }

	inline const std::string &getAnnotation() const { return m_annotation; }
	inline const std::string &getCustomData() const { return m_custom_data; }
	inline const std::string &getSrvInfo() const { return m_srv_info; }

	friend class ::AfContainer;
	friend class ::AfList;

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	enum Flags
	{
		FZombie = 1 << 0,
		FHidden = 1 << 1
	};

	inline void setZombieFlag() { m_flags = m_flags | FZombie; }
	inline bool isZombie() const { return (m_flags & FZombie); } ///< Whether a node is zombie.
	inline bool isHidden() const { return (m_flags & FHidden); } ///< Whether a node is hidden.

	inline void setLocked(bool i_lock) const { m_locked = i_lock; }
	inline bool isLocked() const { return m_locked; }
	inline bool unLocked() const { return !m_locked; }

	inline void setHidden(bool i_hide = true) { m_flags = i_hide ? m_flags | FHidden : m_flags & (~FHidden); }

	virtual inline long long getTimeCreation() const { return 0; }

	void jsonRead(const JSON &i_object, std::string *io_changes = NULL,
				  MonitorContainer *i_monitoring = NULL);
	virtual void v_jsonWrite(std::ostringstream &o_str, int i_type) const;

	inline const std::map<std::string, int32_t> &getRunnigServices() const { return m_running_services; }

  protected:
	virtual void v_readwrite(Msg *msg); ///< Read or write node attributes in message

	virtual void v_priorityChanged(MonitorContainer *i_monitoring);

	void incrementService(const std::string &i_name, int count = 1);
	void decrementService(const std::string &i_name, int count = 1);

	inline void clearRunningServices() { m_running_services.clear(); }

  protected:
	/// Node id, unique for nodes of the same type. It is a position in container where node is stored.
	int32_t m_id;

	/// Node priority. When new node added to container or a priority changed, container sort nodes by
	/// priority.
	uint8_t m_priority;

	/**
	 * @brief Node name. Name is unique for nodes stored in container.
	 * When new node added and a node with the same name is already exists in container,
	 * container change node name by adding a number.
	 */
	std::string m_name;

	mutable bool m_locked; ///< Lock state.

	int64_t m_state; ///< State.
	int64_t m_flags; ///< Flags.

	std::string m_annotation;
	std::string m_custom_data;
	std::string m_srv_info; ///< Some info that server can show to user.

	std::map<std::string, int32_t> m_running_services;
};
} // namespace af
