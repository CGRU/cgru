#ifndef _DISPLAY_HIERARCHY_H
#define _DISPLAY_HIERARCHY_H

#include <fbxfilesdk/fbxfilesdk_def.h>

// Forward declaration.
class FBXFILESDK_NAMESPACE::KFbxScene;

int DisplayHierarchy(FBXFILESDK_NAMESPACE::KFbxScene* pScene, KString &fileName, KStringList &nodesNames);

#endif // #ifndef _DISPLAY_HIERARCHY_H
