#include "polySplitFty.h"

tm_polySplitFty::tm_polySplitFty():
						firstTime(true),
						oldVtxCount(0),
						oldUVsCount(0),
						newVtxCount(0),
						newUVsCount(0),
						fa_loop(true),
						fa_loop_mode(3),
						fa_loop_angle(45),
						fa_maxcount(25000),
						fa_sel(false)
{
}

tm_polySplitFty::~tm_polySplitFty()
{
}

void tm_polySplitFty::setMesh( MObject mesh )
{
	fMesh = mesh;
}

void tm_polySplitFty::setEdgesIds( MIntArray edgesIds )
{
	fSelEdges = edgesIds;
}
