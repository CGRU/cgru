#ifndef tm_polygon_edgestoring_h
#define tm_polygon_edgestoring_h

#include "../definitions.h"

#include <maya/MIntArray.h>

#include <maya/MObject.h>

class tm_polygon_edgestoring
{
public:
	
	tm_polygon_edgestoring();
	virtual ~tm_polygon_edgestoring();

	bool setMesh( MObject object);
	bool calculate( MIntArray &edgesArray );

private:

	MObject meshObject;
	bool objectIsSet;

};

#endif
