#ifndef tm_polySplitFty_h
#define tm_polySplitFty_h

#include "../../../definitions.h"

#include "../polyModifierFty.h"
// General Includes
//
#include <maya/MObject.h>

#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MVectorArray.h>
#include <maya/MPointArray.h>

class tm_polySplitFty : public polyModifierFty
{

public:
				tm_polySplitFty();
	virtual		~tm_polySplitFty();

	void		setMesh( MObject mesh );
	void		setEdgesIds( MIntArray edgesIds );

	// polyModifierFty inherited methods
	//
	MStatus		doIt();

	MPoint averagePos;

	int oldVtxCount;
	int newVtxCount;
	MPointArray splitedPoints_start_N;
	MVectorArray splitedPoints_dir_N;
	MVectorArray splitedPoints_ndir_N;
	MPointArray splitedPoints_start_R;
	MVectorArray splitedPoints_dir_R;
	MVectorArray splitedPoints_ndir_R;

	int oldUVsCount;
	int newUVsCount;
	MFloatArray splitedUVsU_start_N;
	MFloatArray splitedUVsU_dir_N;
	MFloatArray splitedUVsU_ndir_N;
	MFloatArray splitedUVsU_start_R;
	MFloatArray splitedUVsU_dir_R;
	MFloatArray splitedUVsU_ndir_R;
	MFloatArray splitedUVsV_start_N;
	MFloatArray splitedUVsV_dir_N;
	MFloatArray splitedUVsV_ndir_N;
	MFloatArray splitedUVsV_start_R;
	MFloatArray splitedUVsV_dir_R;
	MFloatArray splitedUVsV_ndir_R;

	bool firstTime;

	bool fa_loop;
	int fa_loop_mode;
	double fa_loop_angle;
	long fa_maxcount;
	bool fa_sel;


private:
	// Mesh Node - Note: We only make use of this MObject during a single call of
	//					 the tm_polySplit plugin. It is never maintained and used between
	//					 calls to the plugin as the MObject handle could be invalidated
	//					 between calls to the plugin.
	//
	MObject		fMesh;

	MIntArray fSelEdges;
	unsigned selEdgesCount;

	unsigned edgesCount;
	MIntArray invEdge;

	bool getLoopFromFirst( const int &mode, const double &angleDeg, const int &maxCount);
	bool getRing();

};

#endif
