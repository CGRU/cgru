#ifndef tm_polySplitManip_h
#define tm_polySplitManip_h

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

class tm_polySplitNodeManip : public MPxManipContainer
{
public:
    tm_polySplitNodeManip();
    virtual ~tm_polySplitNodeManip();
    
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

	MPoint edgePos;
	MVector edgeDir;

	MDagPath fNodePath;

public:
    static MTypeId id;
};

#endif
