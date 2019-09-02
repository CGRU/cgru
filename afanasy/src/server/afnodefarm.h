#pragma once

#include "../libafanasy/affarm.h"

#include "afnodesrv.h"

class Action;
class PoolSrv;

class AfNodeFarm: public AfNodeSrv
{
public:
	AfNodeFarm(af::Node * i_node, af::Farm * i_farm, int i_type, PoolSrv * i_parent, const std::string & i_store_dir = "");
	virtual ~AfNodeFarm();

	enum Type {
		TPool,
		TRenderer
	};

	bool canRunService(const std::string & i_service_name, bool i_hasServicesSetup = false) const;

	// On success return true for monitoring/storing
	bool actionFarm(Action & i_action);

protected:
	PoolSrv * m_parent;

private:
	bool hasService(const std::string & i_service_name) const;
	bool isServiceDisabled(const std::string & i_service_name) const;

private:
	af::Farm * m_farm;
	int m_type;
	int m_change_event;
};
