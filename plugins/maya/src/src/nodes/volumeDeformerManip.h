#ifndef VOLUMEDEFORMERMANIP_H
#define VOLUMEDEFORMERMANIP_H

#include "../definitions.h"

#include <maya/MArrayDataBuilder.h>

#include <maya/MPxManipContainer.h> 
#include <maya/MManipData.h>
#include <maya/M3dView.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MPlug.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>

class tm_volumeDeformerManip : public MPxManipContainer
{
public:
    tm_volumeDeformerManip();
    virtual ~tm_volumeDeformerManip();

    static void * creator();
    static MStatus initialize();
    virtual MStatus createChildren();
    virtual MStatus connectToDependNode(const MObject & node);

    virtual void draw(M3dView & view,
                        const MDagPath & path, 
                        M3dView::DisplayStyle style,
                        M3dView::DisplayStatus status);

   MDagPath fDistanceManip;

public:
    static MTypeId id;

private:
   MPoint radiusOffset;
};

#endif
