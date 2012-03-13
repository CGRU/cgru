#ifndef _DISPLAY_ANIMATION_H
#define _DISPLAY_ANIMATION_H

#include <fbxfilesdk/fbxfilesdk_def.h>

// Forward declarations.
class FBXFILESDK_NAMESPACE::KFbxScene;
class FBXFILESDK_NAMESPACE::KFbxNode;

void DisplayAnimation(FBXFILESDK_NAMESPACE::KFbxScene* pScene, KString &fileName, int nodecount, int framesteps, int mode, KStringList &nodesNames);

#endif // #ifndef _DISPLAY_ANIMATION_H
