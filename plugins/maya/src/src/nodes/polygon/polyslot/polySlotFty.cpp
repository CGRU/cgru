#include "polySlotFty.h"

tm_polySlotFty::tm_polySlotFty():
						firstTime(true)
{
//	fSelEdges.clear();
	slotVerticesIds.setSizeIncrement( 100);
	rightFaces.setSizeIncrement( 100);
	polyConnects.setSizeIncrement( 100);
}

tm_polySlotFty::~tm_polySlotFty()
{}

void tm_polySlotFty::setMesh( MObject mesh )
{
	fMesh = mesh;
}

void tm_polySlotFty::setEdgesIds( MIntArray edgesIds )
{
	fSelEdges = edgesIds;
}
