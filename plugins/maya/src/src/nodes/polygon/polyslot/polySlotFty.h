#ifndef tm_polySlotFty_h
#define tm_polySlotFty_h

#include "../../../definitions.h"

#include "../polyModifierFty.h"
// General Includes
//
#include <maya/MObject.h>

#include <maya/MIntArray.h>
#include <maya/MVectorArray.h>
#include <maya/MPointArray.h>

#include <maya/MString.h>

class tm_polySlotFty : public polyModifierFty
{

public:
				tm_polySlotFty();
	virtual		~tm_polySlotFty();

	void		setMesh( MObject mesh );
	void		setEdgesIds( MIntArray edgesIds );

	// polyModifierFty inherited methods
	//
	MStatus		doIt();

	MPoint averagePos;
	MVector averageDir;

	unsigned offsetVerticesCont_a;
	MIntArray offsetVerticesIds_a;
	MVectorArray offsetVerticesDir_a;
	MPointArray offsetVerticesStart_a;

	unsigned offsetVerticesCont_b;
	MIntArray offsetVerticesIds_b;
	MVectorArray offsetVerticesDir_b;
	MPointArray offsetVerticesStart_b;

	bool firstTime;

private:
	// Mesh Node - Note: We only make use of this MObject during a single call of
	//					 the tm_polySlot plugin. It is never maintained and used between
	//					 calls to the plugin as the MObject handle could be invalidated
	//					 between calls to the plugin.
	//
	MObject		fMesh;

	bool validateEdges();

	MIntArray fSelEdges;
	unsigned selEdgesCount;

	MIntArray polyCounts, polyConnects;
	unsigned polyCount;

	MIntArray rightFaces;
	int rightFacesCount;

	bool isFaceConnected( int face);

	int oldVtxCount;
	int newVtxCount;
	int slotVtxCount;
	MIntArray slotVerticesIds;

};

#endif
