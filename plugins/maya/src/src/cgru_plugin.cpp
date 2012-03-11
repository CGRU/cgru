#include "definitions.h"

#include <maya/MArrayDataBuilder.h>
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
// commands:
#include "./commands/command.h"

#include "./commands/randPoint.h"
#include "./commands/view.h"
#include "./commands/reparent.h"
#include "./commands/killNode.h"
#include "./commands/clearOutput.h"

#include "./commands/polygon/polygon.h"
#include "./commands/polygon/polySelect.h"
#include "./commands/polygon/polyModPrep.h"
#include "./commands/polygon/polyExtract.h"

// nodes:

#include "./nodes/kxb.h"
#include "./nodes/kxbci.h"
#include "./nodes/noiseNode.h"
#include "./nodes/randomDeformer.h"
#include "./nodes/geoSphere.h"
#include "./nodes/makehgeo.h"
#include "./nodes/volumeDeformer.h"
#include "./nodes/volumeDeformerManip.h"

#include "./nodes/polygon/transformUV.h"

#include "./nodes/polygon/polyslot/polySlotCmd.h"
#include "./nodes/polygon/polyslot/polySlotNode.h"
#include "./nodes/polygon/polyslot/polySlotManip.h"

#include "./nodes/polygon/polysplit/polySplitCmd.h"
#include "./nodes/polygon/polysplit/polySplitNode.h"
#include "./nodes/polygon/polysplit/polySplitManip.h"
#include "./nodes/polygon/polysplit/polySplitLoopSel.h"

// shading nodes:
#include "./nodes/shading/grade.h"

MStatus initializePlugin( MObject obj )
{

   MStatus   status;
   MFnPlugin plugin( obj, "Timur Hairulin", "8.5 16.05.2007", "Any");
// unloading hybrid mentalray nodes if any

MStringArray miCustomNodesFiles;
MStringArray cgru_miCustomNodesFiles;
bool execmd = false;
if(plugin.findPlugin("Mayatomr") != MObject::kNullObj)
{
   MGlobal::executeCommand ( "miCustomNodesFiles()", miCustomNodesFiles, false, false);
   for(unsigned i = 0; i < miCustomNodesFiles.length(); i++)
   {
      int rindex = miCustomNodesFiles[i].rindex('/') + 1;
      int index =  miCustomNodesFiles[i].rindex('_') - 1;
      if(miCustomNodesFiles[i].substring(rindex, index) == "cgru")
      {
         cgru_miCustomNodesFiles.append(miCustomNodesFiles[i]);
         execmd = true;
      }
   }
   for(unsigned i = 0; i < cgru_miCustomNodesFiles.length(); i++)
   {
      MString command = "mrFactory -unload \""+cgru_miCustomNodesFiles[i]+"\"";
      MGlobal::executeCommand ( command, true, false);
   }
}

   status = plugin.registerCommand( "tm_command", tm_command::creator, tm_command::newSyntax);
   if (!status) { status.perror( "registerCommand tm_command"); return status;}

   status = plugin.registerCommand( "tm_polySelect", tm_polySelect::creator, tm_polySelect::newSyntax);
   if (!status) { status.perror( "registerCommand tm_polySelect"); return status;}

   status = plugin.registerCommand( "tm_randPoint", tm_randPoint::creator, tm_randPoint::newSyntax);
   if (!status) { status.perror( "registerCommand tm_randPoint"); return status;}

   status = plugin.registerCommand( "tm_polygon", tm_polygon::creator, tm_polygon::newSyntax);
   if (!status) { status.perror( "registerCommand tm_polygon"); return status;}

   status = plugin.registerCommand( "tm_view", tm_view::creator, tm_view::newSyntax);
   if (!status) { status.perror( "registerCommand tm_view"); return status;}

   status = plugin.registerCommand( "tm_reparent", tm_reparent::creator, tm_reparent::newSyntax);
   if (!status) { status.perror( "registerCommand tm_reparent"); return status;}

   status = plugin.registerCommand( "tm_killNode", tm_killNode::creator, tm_killNode::newSyntax);
   if (!status) { status.perror( "registerCommand tm_killNode"); return status;}

   status = plugin.registerCommand( "tm_polyModPrep", tm_polyModPrep::creator, tm_polyModPrep::newSyntax);
   if (!status) { status.perror( "registerCommand tm_polyModPrep"); return status;}

   status = plugin.registerCommand( "tm_clearOutput", tm_clearOutput::creator, tm_clearOutput::newSyntax);
   if (!status) { status.perror( "registerCommand tm_clearOutput"); return status;}

   status = plugin.registerCommand( "tm_polyExtract", tm_polyExtract::creator, tm_polyExtract::newSyntax);
   if (!status) { status.perror( "registerCommand tm_polyExtract"); return status;}


   status = plugin.registerNode( "tm_kxb", tm_kxb::id, tm_kxb::creator, tm_kxb::initialize );
   if (!status) { status.perror( "registerNode tm_kxb"); return status;}

   status = plugin.registerNode( "tm_kxbci", tm_kxbci::id, tm_kxbci::creator, tm_kxbci::initialize );
   if (!status) { status.perror( "registerNode tm_kxbci"); return status;}

   status = plugin.registerNode( "tm_noisePerlin", tm_noisePerlin::id, tm_noisePerlin::creator, tm_noisePerlin::initialize, MPxNode::kDeformerNode );
   if (!status) { status.perror( "registerNode tm_noisePerlin"); return status;}

   status = plugin.registerNode( "tm_randDef", tm_randDef::id, tm_randDef::creator, tm_randDef::initialize, MPxNode::kDeformerNode );
   if (!status) { status.perror( "registerNode tm_randDef"); return status;}

   status = plugin.registerNode( "tm_volumeDeformer", tm_volumeDeformer::id, tm_volumeDeformer::creator, tm_volumeDeformer::initialize, MPxNode::kDeformerNode );
   if (!status) { status.perror( "registerNode tm_volumeDeformer"); return status;}
   status = plugin.registerNode( "tm_volumeDeformerManip", tm_volumeDeformerManip::id, &tm_volumeDeformerManip::creator, &tm_volumeDeformerManip::initialize, MPxNode::kManipContainer);
   if (!status) { status.perror( "registerNode tm_volumeDeformerManip"); return status;}

   status = plugin.registerNode( "tm_makeGeoSphere", tm_makeGeoSphere::id, &tm_makeGeoSphere::creator, &tm_makeGeoSphere::initialize);
   if (!status) { status.perror( "registerNode tm_makeGeoSphere"); return status;}

   status = plugin.registerNode( "tm_makeHgeo", tm_makeHgeo::id, &tm_makeHgeo::creator, &tm_makeHgeo::initialize);
   if (!status) { status.perror( "registerNode tm_makeHgeo"); return status;}

   status = plugin.registerNode( "tm_transformUV", tm_transformUV::id, tm_transformUV::creator, tm_transformUV::initialize );
   if (!status) { status.perror( "registerNode tm_transformUV"); return status;}

   status = plugin.registerCommand( "tm_polySlot", tm_polySlot::creator);
   if (!status) { status.perror( "registerCommand tm_polySlot"); return status;}
   status = plugin.registerNode( "tm_polySlotNode", tm_polySlotNode::id, tm_polySlotNode::creator, tm_polySlotNode::initialize );
   if (!status) { status.perror( "registerNode tm_polySlotNode"); return status;}
   status = plugin.registerNode( "tm_polySlotNodeManip", tm_polySlotNodeManip::id, &tm_polySlotNodeManip::creator, &tm_polySlotNodeManip::initialize, MPxNode::kManipContainer); 
   if (!status) { status.perror( "registerNode tm_polySlotNodeManip"); return status;}

   status = plugin.registerCommand( "tm_polySplit", tm_polySplit::creator, tm_polySplit::newSyntax);
   if (!status) { status.perror( "registerCommand tm_polySplit"); return status;}
   status = plugin.registerNode( "tm_polySplitNode", tm_polySplitNode::id, tm_polySplitNode::creator, tm_polySplitNode::initialize );
   if (!status) { status.perror( "registerNode tm_polySplitNode"); return status;}
   status = plugin.registerNode( "tm_polySplitNodeManip", tm_polySplitNodeManip::id, &tm_polySplitNodeManip::creator, &tm_polySplitNodeManip::initialize, MPxNode::kManipContainer); 
   if (!status) { status.perror( "registerNode tm_polySplitNodeManip"); return status;}
   status = plugin.registerCommand( "tm_polySplitLoopSel", tm_polySplitLoopSel::creator, tm_polySplitLoopSel::newSyntax);
   if (!status) { status.perror( "registerCommand tm_polySplitLoopSel"); return status;}

   MString classification;
   classification = "utility/color";
   status = plugin.registerNode( "grade_tm", grade_tm::id, grade_tm::creator, grade_tm::initialize, MPxNode::kDependNode, &classification);
   if (!status) { status.perror( "registerNode grade_tm"); return status;}

// loading hybrid mentalray nodes if any
if(execmd)
{
   for(unsigned i = 0; i < cgru_miCustomNodesFiles.length(); i++)
   {
      MString command = "mrFactory -load \""+cgru_miCustomNodesFiles[i]+"\"";
      MGlobal::executeCommand ( command, true, false);
   }
}
/**/
   return status;
}

MStatus uninitializePlugin( MObject obj)
{
   MStatus   status;
   MFnPlugin plugin( obj );

   status = plugin.deregisterCommand( "tm_command" );
   if (!status) { status.perror("deregisterCommand tm_command"); return status;}

   status = plugin.deregisterCommand( "tm_polySelect" );
   if (!status) { status.perror("deregisterCommand tm_polySelect"); return status;}

   status = plugin.deregisterCommand( "tm_randPoint" );
   if (!status) { status.perror("deregisterCommand tm_randPoint"); return status;}

   status = plugin.deregisterCommand( "tm_polygon" );
   if (!status) { status.perror("deregisterCommand tm_polygon"); return status;}

   status = plugin.deregisterCommand( "tm_view" );
   if (!status) { status.perror("deregisterCommand tm_view"); return status;}

   status = plugin.deregisterCommand( "tm_reparent" );
   if (!status) { status.perror("deregisterCommand tm_reparent"); return status;}

   status = plugin.deregisterCommand( "tm_killNode" );
   if (!status) { status.perror("deregisterCommand tm_killNode"); return status;}

   status = plugin.deregisterCommand( "tm_polyModPrep" );
   if (!status) { status.perror("deregisterCommand tm_polyModPrep"); return status;}

   status = plugin.deregisterCommand( "tm_clearOutput" );
   if (!status) { status.perror("deregisterCommand tm_clearOutput"); return status;}

   status = plugin.deregisterCommand( "tm_polyExtract" );
   if (!status) { status.perror("deregisterCommand tm_polyExtract"); return status;}


   status = plugin.deregisterNode( tm_kxb::id );
   if (!status) { status.perror("deregisterNode tm_kxb"); return status;}

   status = plugin.deregisterNode( tm_kxbci::id );
   if (!status) { status.perror("deregisterNode tm_kxbci"); return status;}

   status = plugin.deregisterNode( tm_noisePerlin::id );
   if (!status) { status.perror("deregisterNode tm_noisePerlin"); return status;}

   status = plugin.deregisterNode( tm_randDef::id );
   if (!status) { status.perror("deregisterNode tm_randDef"); return status;}

   status = plugin.deregisterNode( tm_volumeDeformer::id );
   if (!status) { status.perror("deregisterNode tm_volumeDeformer"); return status;}
   status = plugin.deregisterNode( tm_volumeDeformerManip::id );
   if (!status) { status.perror("deregisterNode tm_volumeDeformerManip"); return status;}

   status = plugin.deregisterNode( tm_makeGeoSphere::id );
   if (!status) { status.perror("deregisterNode tm_makeGeoSphere"); return status;}

   status = plugin.deregisterNode( tm_makeHgeo::id );
   if (!status) { status.perror("deregisterNode tm_makeHgeo"); return status;}

   status = plugin.deregisterNode( tm_transformUV::id );
   if (!status) { status.perror("deregisterNode tm_transformUV"); return status;}

   status = plugin.deregisterCommand( "tm_polySlot" );
   if (!status) { status.perror("deregisterCommand tm_polySlot"); return status;}
   status = plugin.deregisterNode( tm_polySlotNode::id );
   if (!status) { status.perror("deregisterNode tm_polySlotNode"); return status;}
   status = plugin.deregisterNode( tm_polySlotNodeManip::id );
   if (!status) { status.perror("deregisterNode tm_polySlotNodeManip"); return status;}

   status = plugin.deregisterCommand( "tm_polySplit" );
   if (!status) { status.perror("deregisterCommand tm_polySplit"); return status;}
   status = plugin.deregisterNode( tm_polySplitNode::id );
   if (!status) { status.perror("deregisterNode tm_polySplitNode"); return status;}
   status = plugin.deregisterNode( tm_polySplitNodeManip::id );
   if (!status) { status.perror("deregisterNode tm_polySplitNodeManip"); return status;}
   status = plugin.deregisterCommand( "tm_polySplitLoopSel" );
   if (!status) { status.perror("deregisterCommand tm_polySplitLoopSel"); return status;}

   status = plugin.deregisterNode( grade_tm::id );
   if (!status) { status.perror("deregisterNode grade_tm"); return status;}
/**/
   return status;
}
