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

#ifndef _polyModifierFty
#define _polyModifierFty

// 
// File: polyModifierFty.h
//
// polyModifier Factory: polyModifierFty
//
// Author: Lonnie Li
//
// Overview:
//
//		The polyModifierFty class is the main workhorse of the polyModifierCmd operation.
//		It is here that the actual operation is implemented. The idea of the factory is
//		to create a single implementation of the modifier that can be reused in more than
//		one place.
//
//		As such, the details of the factory are quite simple. Each factory contains a doIt()
//		method which should be overridden. This is the method which will be called by the
//		node and the command when a modifier is requested.
//
// How to use:
//
//		1) Create a factory derived from polyModifierFty
//		2) Add any input methods and members to the factory
//		3) Override the polyModifierFty::doIt() method
//
//			(a) Retrieve the inputs from the class
//			(b) Process the inputs
//			(c) Perform the modifier
//
//

// General Includes
//
#include <maya/MStatus.h>

class polyModifierFty
{
public:
						polyModifierFty();
	virtual				~polyModifierFty();

	// Pure virtual doIt()
	//
	virtual MStatus		doIt() = 0;
};

#endif
