//-
// ==========================================================================
// Copyright (C) 1995 - 2005 Alias Systems Corp. and/or its licensors.  All 
// rights reserved. 
// 
// The coded instructions, statements, computer programs, and/or related 
// material (collectively the "Data") in these files are provided by Alias 
// Systems Corp. ("Alias") and/or its licensors for the exclusive use of the 
// Customer (as defined in the Alias Software License Agreement that 
// accompanies this Alias software). Such Customer has the right to use, 
// modify, and incorporate the Data into other products and to distribute such 
// products for use by end-users.
//  
// THE DATA IS PROVIDED "AS IS".  ALIAS HEREBY DISCLAIMS ALL WARRANTIES 
// RELATING TO THE DATA, INCLUDING, WITHOUT LIMITATION, ANY AND ALL EXPRESS OR 
// IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE. IN NO EVENT SHALL ALIAS BE LIABLE FOR ANY DAMAGES 
// WHATSOEVER, WHETHER DIRECT, INDIRECT, SPECIAL, OR PUNITIVE, WHETHER IN AN 
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, OR IN EQUITY, 
// ARISING OUT OF ACCESS TO, USE OF, OR RELIANCE UPON THE DATA.
// ==========================================================================
//+

#include "../../definitions.h"

#ifndef _polyModifierNode
#define _polyModifierNode

#include <maya/MArrayDataBuilder.h>

// 
// File: polyModifierNode.h
//
// Dependency Graph Node: polyModifierNode
//
// Author: Lonnie Li
//
// Overview:
//
//		The polyModifierNode class is a intermediate class used by polyModifierCmd to
//		modify a polygonal object in Maya. The purpose of the polyModifierNode is to 
//		generalize a node that can receive an input mesh object, modify the mesh and
//		return the modified mesh.
//
//		polyModifierNode is an abstraction which does not need to know about the DG
//		and simply needs to know about the process outlined above. polyModifierCmd
//		manages when and how this node will be used.
//
//		Each polyModifierNode is recommended to contain an instance of a polyModifierFty
//		which will do the actual work, leaving only the node setup and attribute
//		associations to the node. The recommended structure of a polyModifierNode is
//		as follows:
//
//              _____________
//             /        ___  \
//           /         /   \   \
//        O |   Node  | Fty |   | O
//        |  \         \___/   /  |
//        |    \_____________/    |
//      inMesh                 outMesh
//
//
//		The purpose of the node is to simply define the attributes (inputs and outputs) of
//		the node and associate which attribute affects each other. This is basic node setup
//		for a DG node. Using the above structure, the node's inherited "compute()" method
//		(from MPxNode) should retrieve the inputs and pass the appropriate data down to the
//		polyModifierFty for processing.
//
//
// How to use: 
//
//		(1) Create a class derived from polyModifierNode
//		(2) Define and associate inMesh and outMesh attributes (inMesh --> affects --> outMesh)
//		(3) Add any additional attributes specific to the derived node and setup associations
//		(4) Define an instance of your specific polyModifierFty to perform the operation on the node
//		(5) Override the MPxNode::compute() method
//		(6) Inside compute():
//
//			(a) Retrieve input attributes
//			(b) Use inputs to setup your factory to operate on the given mesh
//			(c) Call the factory's inherited doIt() method
//
//

// Proxies
//
#include <maya/MPxNode.h>

class polyModifierNode : public MPxNode
{
public:
						polyModifierNode();
	virtual				~polyModifierNode(); 

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static  MObject		inMesh;
	static  MObject		outMesh;
};

#endif
