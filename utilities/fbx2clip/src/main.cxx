#include <fbxsdk.h>

#include "Common/Common.h"
#include "DisplayAnimation.h"
#include "DisplayHierarchy.h"

#include <fbxfilesdk/fbxfilesdk_nsuse.h>

void DisplayMetaData(KFbxScene* pScene);

double framerate_mult;

int main(int argc, char** argv)
{
   if( argc < 4)
   {
      printf("\nUsage: %s <FBX file name> <framesteps> <mode=[trsmi]> <nodeslist filename>\n", argv[0]);
      exit(1);
   }

   KFbxSdkManager* lSdkManager = NULL;
   KFbxScene* lScene = NULL;
   bool lResult;

   KString hierarchyFileName;
   //
   // Prepare the FBX SDK.
   InitializeSdkObjects(lSdkManager, lScene);
   //
   // take a FBX file as an argument and load the scene
      printf("\nFile: %s\n", argv[1]);
   lResult = LoadScene(lSdkManager, lScene, argv[1]);
   //
   // take a flamesteps as an argument.
   int flamesteps = atoi(argv[2]);
   if( flamesteps < 1 )
   {
      flamesteps = 2;
      printf("\nInvalid framesteps, using default = %d\n", flamesteps);
   }
   //
   // take mode as an argument.
   int mode = 0;
   switch( argv[3][0])
   {
      case 't':
      {
         mode  = 0;
         printf("\nGetting world translates from all objects in scene.\n");
         break;
      }
      case 'r':
      {
         mode  = 1;
         printf("\nGetting world translates and rotates from all objects in scene.\n");
         break;
      }
      case 's':
      {
         mode  = 2;
         printf("\nGetting world translates,rotates and scales from all objects in scene.\n");
         break;
      }
      case 'm':
      {
         mode  = 3;
         printf("\nGetting global transformation matrix from all objects in scene.\n");
         break;
      }
      case 'i':
      {
         mode  = 4;
         printf("\nPrint files (fbx and clip) info only.\n");
         break;
      }
      default:
      {
         printf("\nInvalid mode=\"%s\", extractiong translations only by default.\n", argv[3]);
         printf("<mode=trsmi>: transtales, translate+rotate, translate+rotate+scale, matrix and info only.\n\n");
         break;
      }
   }
   //
   // setup filename
   KString fileName;
   fileName = argv[1];
   //
   // setup nodes names list
   KStringList nodesNames;
   if( argc > 4)
   {
      printf("Using nodes names file \"%s\".\n", argv[4]);
      FILE *listFile = fopen( argv[4], "r" );
      if( listFile == NULL )
      {
         perror("Can't open file");
      }
      else
      {
         const int NAME_MAX = 256;
         char line[NAME_MAX];
         while ( fgets( line, NAME_MAX, listFile) != NULL)
         {
            line[ strlen( line) - 1] = '\0';
            char *name = line;
            while( name[0] == ' ') name++;
            nodesNames.Add( name);
         }
         fclose( listFile);
         fileName += ".";
         fileName += argv[4];
      }
   }

   //
   //
   if(lResult == false)
   {
      printf("\nAn error occured while loading the scene...\n");
      return 0;
   }
   else
   {
      int nodecount = DisplayHierarchy(lScene, fileName, nodesNames);
      DisplayAnimation(lScene, fileName, nodecount, flamesteps, mode, nodesNames);
   }

   //
   // Destroy all objects created by the FBX SDK.
   DestroySdkObjects(lSdkManager);

   return 0;
}
