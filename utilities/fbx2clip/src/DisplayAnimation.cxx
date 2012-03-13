#include <fbxsdk.h>

#include <stdio.h>

#include <fbxfilesdk/fbxfilesdk_nsuse.h>

void DisplayAnimation( KFbxNode* pNode,
               int  mode,
               KStringList &nodesNames,
               bool isSwitcher = false);
void DisplayChannels( KFbxNode* pNode,
               int  mode,
               bool isSwitcher);
void DisplayKeys( KFbxNode* pNode,
               int type);

FILE *stream_a;
KTime timestart;
KTime timestop;
KTime interval;

void DisplayAnimation(KFbxScene* pScene, KString &fileName, int nodecount, int framesteps, int mode, KStringList &nodesNames)
{

   KArrayTemplate<KString*> lTakeNameArray;
   int i;

   pScene->FillTakeNameArray(lTakeNameArray);

   for(i = 0; i < lTakeNameArray.GetCount(); i++)
   {
      // It's useless to display the default animation because it is always empty.
      if(lTakeNameArray.GetAt(i)->Compare( KFBXTAKENODE_DEFAULT_NAME) == 0)
      {
         continue;
      }

      FILE *stream_b = fopen( (fileName + ".info").Buffer(), "w" );

      pScene->SetCurrentTake(lTakeNameArray.GetAt(i)->Buffer());
      KString takename = pScene->GetCurrentTakeName();
         printf("Take Name: ");        fprintf( stream_b, "Take Name: ");
         printf(takename);             fprintf( stream_b, takename);
         printf("\n");                 fprintf( stream_b, "\n");

      KFbxTakeInfo *takeinfo = pScene->GetTakeInfo( takename);
      KTimeSpan timespan = takeinfo->mLocalTimeSpan;
//
//    get start and stop time ///////////////////////////////////////
      timestart = timespan.GetStart();
      timestop  = timespan.GetStop();
         char buffer[256];
         printf(            "frame start = %s\n", timestart.GetTimeString( buffer, 4));
         printf(            "frame stop  = %s\n", timestop.GetTimeString(  buffer, 4));
         fprintf( stream_b, "frame start = %s\n", timestart.GetTimeString( buffer, 4));
         fprintf( stream_b, "frame stop  = %s\n", timestop.GetTimeString(  buffer, 4));
//
//    get frame rate and clip rate ///////////////////////////////////////
      double framerate = timestart.GetFrameRate( KTime::eDEFAULT_MODE);
         printf("framerate   = %f\n", framerate);     fprintf( stream_b, "framerate   = %f\n", framerate);
         printf("framesteps  = %d\n", framesteps);    fprintf( stream_b, "framesteps  = %d\n", framesteps);
      int cliprate = int(framerate*framesteps);
         printf("cliprate    = %d\n", cliprate);      fprintf( stream_b, "cliprate    = %d\n", cliprate);
      interval.SetSecondDouble( 1.0 / cliprate);
         printf(            "interval    = %f\n", interval.GetSecondDouble());
         fprintf( stream_b, "interval    = %f\n", interval.GetSecondDouble());

//
//   get tracks lenght
      int tracklength = 0;
      for( KTime ktime = timestart; ktime <= timestop; ktime += interval) tracklength ++;
         printf("tracklength = %d\n", tracklength);   fprintf( stream_b, "tracklength = %d\n", tracklength);
         printf("nodecount   = %d\n", nodecount);     fprintf( stream_b, "nodecount   = %d\n", nodecount);

//
//   exiting if mode == info
      fclose( stream_b );
      if( mode == 4) break;

      int traks = nodecount;
      switch( mode)
      {
         case 0:  { traks *=  3; break; }
         case 1:  { traks *=  6; break; }
         case 2:  { traks *=  9; break; }
         case 3:  { traks *= 16; break; }
      }
         printf("traks       = %d\n", traks);

      stream_a = fopen( (fileName + ".clip").Buffer(), "w" );
      fprintf(stream_a, "{\n");
      fprintf(stream_a, "rate = %d\n", cliprate);
      fprintf(stream_a, "start = 0\n");
      fprintf(stream_a, "tracklength = %d\n", tracklength);
      fprintf(stream_a, "tracks = %d\n", traks);

      DisplayAnimation( pScene->GetRootNode(), mode, nodesNames);
      fprintf(stream_a, "}\n");
      fclose( stream_a );

      break;
   }

   DeleteAndClear(lTakeNameArray);

}

void DisplayAnimation( KFbxNode* pNode, int mode, KStringList &nodesNames, bool isSwitcher)
{
//   KFbxTakeNode* lCurrentTakeNode = pNode->GetCurrentTakeNode();
   char* lCurrentTakeName = pNode->GetCurrentTakeNodeName();
   char* lDefaultTakeName = pNode->GetTakeNodeName(0);
   int lModelCount;

   // Display nothing if the current take node points to default values.
//   if( lCurrentTakeNode && lCurrentTakeNode != pNode->GetDefaultTakeNode())
   if( lCurrentTakeName && lCurrentTakeName != lDefaultTakeName)
   {
      int namesCount = nodesNames.GetCount();
      if( namesCount > 0)
      {
         KString nodeName = pNode->GetName();
         for( int i = 0; i < namesCount; i++)
         {
            if( nodesNames[i] == nodeName)
            {
               DisplayChannels(pNode, mode, isSwitcher);
            }
         }
      }
      else
      {
         DisplayChannels(pNode, mode, isSwitcher);
      }
   }

   for( lModelCount = 0; lModelCount < pNode->GetChildCount(); lModelCount++)
   {
        DisplayAnimation(pNode->GetChild(lModelCount), mode, nodesNames, isSwitcher);
   }
}

void DisplayChannels( KFbxNode* pNode, int mode, bool isSwitcher)
{
   if ( isSwitcher) return;

   static char *channelname[25] = {
                                   "tx","ty","tz",
                                   "rx","ry","rz",
                                   "sx","sy","sz",

                                   "m11","m12","m13","m14",
                                   "m21","m22","m23","m24",
                                   "m31","m32","m33","m34",
                                   "m41","m42","m43","m44"};

   KString nodeName = pNode->GetName();
   nodeName.FindAndReplace( "Model::", "");
   nodeName.FindAndReplace( " ", "_");
   nodeName = "{\nname = " + nodeName;

   KString lOutputString;

   int start;
   int finish;
   switch( mode)
   {
      case 0: { start  = 0; finish =  3; break; }
      case 1: { start  = 0; finish =  6; break; }
      case 2: { start  = 0; finish =  9; break; }
      case 3: { start  = 9; finish = 25; break; }
   }

   for( int i = start; i < finish; i++)
   {
      lOutputString = nodeName;
      lOutputString += ":";
      lOutputString += channelname[i];
      lOutputString += "\ndata =";
      fprintf( stream_a, lOutputString);
      DisplayKeys( pNode, i);
      fprintf( stream_a, "\n}\n");
   }
}

void DisplayKeys( KFbxNode* pNode, int type)
{
   for( KTime ktime = timestart; ktime <= timestop; ktime += interval)
   {
      KFbxXMatrix kmatrix = pNode->GetGlobalFromCurrentTake( ktime);
      KString str = " ";
      switch( type)
      {
         case  0: { fprintf( stream_a, " %f", (kmatrix.GetT())[0]); break; }
         case  1: { fprintf( stream_a, " %f", (kmatrix.GetT())[1]); break; }
         case  2: { fprintf( stream_a, " %f", (kmatrix.GetT())[2]); break; }

         case  3: { fprintf( stream_a, " %f", (kmatrix.GetR())[0]); break; }
         case  4: { fprintf( stream_a, " %f", (kmatrix.GetR())[1]); break; }
         case  5: { fprintf( stream_a, " %f", (kmatrix.GetR())[2]); break; }

         case  6: { fprintf( stream_a, " %f", (kmatrix.GetS())[0]); break; }
         case  7: { fprintf( stream_a, " %f", (kmatrix.GetS())[1]); break; }
         case  8: { fprintf( stream_a, " %f", (kmatrix.GetS())[2]); break; }


         case  9: { fprintf( stream_a, " %f",  kmatrix.Get( 0, 0)); break; }
         case 10: { fprintf( stream_a, " %f",  kmatrix.Get( 0, 1)); break; }
         case 11: { fprintf( stream_a, " %f",  kmatrix.Get( 0, 2)); break; }
         case 12: { fprintf( stream_a, " %f",  kmatrix.Get( 0, 3)); break; }

         case 13: { fprintf( stream_a, " %f",  kmatrix.Get( 1, 0)); break; }
         case 14: { fprintf( stream_a, " %f",  kmatrix.Get( 1, 1)); break; }
         case 15: { fprintf( stream_a, " %f",  kmatrix.Get( 1, 2)); break; }
         case 16: { fprintf( stream_a, " %f",  kmatrix.Get( 1, 3)); break; }

         case 17: { fprintf( stream_a, " %f",  kmatrix.Get( 2, 0)); break; }
         case 18: { fprintf( stream_a, " %f",  kmatrix.Get( 2, 1)); break; }
         case 19: { fprintf( stream_a, " %f",  kmatrix.Get( 2, 2)); break; }
         case 20: { fprintf( stream_a, " %f",  kmatrix.Get( 2, 3)); break; }

         case 21: { fprintf( stream_a, " %f",  kmatrix.Get( 3, 0)); break; }
         case 22: { fprintf( stream_a, " %f",  kmatrix.Get( 3, 1)); break; }
         case 23: { fprintf( stream_a, " %f",  kmatrix.Get( 3, 2)); break; }
         case 24: { fprintf( stream_a, " %f",  kmatrix.Get( 3, 3)); break; }
      }
   }
}
