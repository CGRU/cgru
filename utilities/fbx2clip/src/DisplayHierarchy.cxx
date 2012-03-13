#include <fbxsdk.h>

#include <stdio.h>

#include <fbxfilesdk/fbxfilesdk_nsuse.h>

// Local functions prototype.
int DisplayHierarchy(KFbxNode* pNode, int pDepth, KStringList &nodesNames);

FILE *stream_h;
int DisplayHierarchy(KFbxScene* pScene, KString &fileName, KStringList &nodesNames)
{
   int i;
   KFbxNode* lRootNode = pScene->GetRootNode();

   stream_h = fopen( (fileName+".hierarchy").Buffer(), "w" );

   int nodeCount = 0;

   for( i = 0; i < lRootNode->GetChildCount(); i++)
   {
      nodeCount = DisplayHierarchy(lRootNode->GetChild(i), 0, nodesNames);
   }

   fclose( stream_h );

   return nodeCount;
}

int DisplayHierarchy(KFbxNode* pNode, int pDepth, KStringList &nodesNames)
{
   static int nodeCount = 0;

   KString lString;

   for( int i = 0; i < pDepth; i++)
   {
      lString += "   ";
   }

   KString nodeName = pNode->GetName();

   int namesCount = nodesNames.GetCount();
   if( namesCount > 0)
   {
      for( int i = 0; i < namesCount; i++)
      {
         if( nodesNames[i] == nodeName)
         {
            lString+="*";
            nodeCount++;
         }
      }
   }
   else
   {
      nodeCount++;
   }

   lString += nodeName + "\n";

   fprintf(stream_h, lString.Buffer());

   for( int i = 0; i < pNode->GetChildCount(); i++)
   {
      DisplayHierarchy(pNode->GetChild(i), pDepth + 1, nodesNames);
   }

   return nodeCount;
}
