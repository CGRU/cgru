#include "itemfarm.h"

#include "listrenders.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ItemFarm::ItemFarm(af::Node * i_afnode, Item::EType i_type, const CtrlSortFilter * i_ctrl_sf):
	ItemNode(i_afnode, i_type, i_ctrl_sf),
	m_depth(0)
{
}

ItemFarm::~ItemFarm()
{
}

void ItemFarm::updateFarmValues(af::Farm * i_affarm)
{
	// Grab services:
	m_services.clear();
	for (const std::string & s : i_affarm->m_services)
		m_services.append(afqt::stoq(s));

	// Grab disabled services:
	m_services_disabled.clear();
	for (const std::string & s : i_affarm->m_services_disabled)
		m_services_disabled.append(afqt::stoq(s));
}

void ItemFarm::setDepth(int i_depth)
{
	m_depth = i_depth;
	m_margin_left = ms_DepthOffset * m_depth;
}

