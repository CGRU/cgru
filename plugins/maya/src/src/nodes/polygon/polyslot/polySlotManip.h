#ifndef tm_polySlotManip_h
#define tm_polySlotManip_h

#include "../../../definitions.h"

#include <maya/MArrayDataBuilder.h>

#include <maya/MPxManipContainer.h> 
#include <maya/MManipData.h>
#include <maya/M3dView.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>

class tm_polySlotNodeManip : public MPxManipContainer
{
public:
    tm_polySlotNodeManip();
    virtual ~tm_polySlotNodeManip();
    
    static void * creator();
    static MStatus initialize();
    virtual MStatus createChildren();
    virtual MStatus connectToDependNode(const MObject & node);

    virtual void draw(M3dView & view, 
					  const MDagPath & path, 
					  M3dView::DisplayStyle style,
					  M3dView::DisplayStatus status);
	MManipData startPointCallback(unsigned index) const;
	MVector nodeTranslation() const;

    MDagPath fDistanceManip;
    MDagPath fDirectionManip;

	MPoint edgesPos;
	MVector edgesDir;

	MDagPath fNodePath;

public:
    static MTypeId id;
};

#endif
