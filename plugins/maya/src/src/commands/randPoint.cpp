#include "randPoint.h"

tm_randPoint::~tm_randPoint()
{
	if(!helpMode)
	{
		delete [] newPointsArrays;
		delete [] oldPointsArrays;
	}
}

void* tm_randPoint::creator()
{
	return new tm_randPoint;
}

const char *tm_randPoint::amp_Flag = "-amp";
const char *tm_randPoint::amp_LongFlag = "-amplitude";
const char *tm_randPoint::ampX_Flag = "-ax";
const char *tm_randPoint::ampX_LongFlag = "-amplitudeX";
const char *tm_randPoint::ampY_Flag = "-ay";
const char *tm_randPoint::ampY_LongFlag = "-amplitudeY";
const char *tm_randPoint::ampZ_Flag = "-az";
const char *tm_randPoint::ampZ_LongFlag = "-amplitudeZ";
const char *tm_randPoint::seed_Flag = "-s";
const char *tm_randPoint::seed_LongFlag = "-seed";
const char *tm_randPoint::obj_Flag = "-obj";
const char *tm_randPoint::obj_LongFlag = "-object";
const char *tm_randPoint::help_Flag = "-h";
const char *tm_randPoint::help_LongFlag = "-help";


MSyntax tm_randPoint::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( amp_Flag, amp_LongFlag, MSyntax::kDouble);
	syntax.addFlag( ampX_Flag, ampX_LongFlag, MSyntax::kDouble);
	syntax.addFlag( ampY_Flag, ampY_LongFlag, MSyntax::kDouble);
	syntax.addFlag( ampZ_Flag, ampZ_LongFlag, MSyntax::kDouble);
	syntax.addFlag( seed_Flag, seed_LongFlag, MSyntax::kLong);
	syntax.addFlag( obj_Flag, obj_LongFlag, MSyntax::kString);
	syntax.addFlag( help_Flag, help_LongFlag, MSyntax::kNoArg);
	return syntax;
}

MStatus tm_randPoint::doIt( const MArgList& args )
{
	MStatus stat = MS::kSuccess;
	double amp = 1.0;
	double amp_x = 1.0;
	double amp_y = 1.0;
	double amp_z = 1.0;
	unsigned int seed = 0;
	bool useAxisAmp = false;
	bool exactObject = false;
	unsigned int num_meshes;
	MString objMString;
	helpMode = false;

	MArgDatabase argData( syntax(), args);
	if(argData.isFlagSet( help_Flag))
	{
		helpMode = true;
		appendToResult( "\n// tm_polygon randPoint usage:\n");
		appendToResult( "//    Command operates with specified object with \"-obj\" flag,\n");
		appendToResult( "//       if this flag is not set, command use selected poly objects or poly vertices.\n");
		appendToResult( "//    Synopsis: tm_polygon [flags].\n");
		appendToResult( "//    Type \"help tm_randPoint\" to query flags.\n");
		return MS::kSuccess;
	}
	if(argData.isFlagSet( amp_Flag))
		argData.getFlagArgument( amp_Flag, 0, amp);
	if(argData.isFlagSet( ampX_Flag))
	{
		argData.getFlagArgument( ampX_Flag, 0, amp_x);
		useAxisAmp = true;
	}
	if(argData.isFlagSet( ampY_Flag))
	{
		argData.getFlagArgument( ampY_Flag, 0, amp_y);
		useAxisAmp = true;
	}
	if(argData.isFlagSet( ampZ_Flag))
	{
		argData.getFlagArgument( ampZ_Flag, 0, amp_z);
		useAxisAmp = true;
	}
	if(argData.isFlagSet( seed_Flag))
		argData.getFlagArgument( seed_Flag, 0, seed);
	if(argData.isFlagSet( obj_Flag))
	{
		exactObject = true;
		argData.getFlagArgument( obj_Flag, 0, objMString);
	}

   double randMax = (double)RAND_MAX;
   double halfRandMax = 0.5 * randMax;


	unsigned i;
    if( exactObject)
	{
		selVtxMode = false;
		MSelectionList m_selList;
		MDagPath m_dagPath;
		stat = m_selList.add( objMString);
		if (!stat){stat.perror("###1 invalid obect path string");return stat;}
		stat = m_selList.getDagPath( 0, m_dagPath);
		if (!stat){stat.perror("###2 invalid obect path string");return stat;}
		stat = res_MDagPathArray.append( m_dagPath);
		if (!stat){stat.perror("###3 invalid obect path string");return stat;}
	}
	else
	{
		MSelectionList curSel;
		MGlobal::getActiveSelectionList( curSel);
		unsigned int numSelected = curSel.length();
		MItSelectionList iter( curSel, MFn::kMeshVertComponent);
		if (iter.isDone())
		{
			selVtxMode = false;
			MItDag::TraversalType traversalType = MItDag::kBreadthFirst;
			MFn::Type filter = MFn::kMesh;
			MItDag mit_dag( traversalType, filter, &stat);
			for( i = 0; i < numSelected; i++ )
			{
				MDagPath sel_dagPath;
				curSel.getDagPath( i, sel_dagPath);
				stat = mit_dag.reset( sel_dagPath,  traversalType, filter);
				if ( !stat) { stat.perror("MItDag constructor");return stat;}
				for ( ; !mit_dag.isDone(); mit_dag.next() )
				{
					MDagPath m_dagPath;
					stat = mit_dag.getPath(m_dagPath);
					if ( !stat ) { stat.perror("MItDag::getPath error"); continue;}
					stat = res_MDagPathArray.append( m_dagPath);
					if ( !stat ) { stat.perror("MDagPathArray.append error"); continue;}
				}
			}
		}
		else
		{
			newPointsArrays = new MPointArray[1];
			oldPointsArrays = new MPointArray[1];
			selVtxMode = true;
			MObject component;
			MDagPath selVtx_dagPath;
			for ( ; !iter.isDone(); iter.next() )
			{
				iter.getDagPath(selVtx_dagPath, component);
				MItMeshVertex vertexIter( selVtx_dagPath, component );
				for ( ; !vertexIter.isDone(); vertexIter.next() )
				{
					MPoint oldPoint = vertexIter.position(MSpace::kObject );
					MPoint newPoint;
					newPoint.x = oldPoint.x + ((halfRandMax - (double)rand()) / randMax) * amp * amp_x;
					newPoint.y = oldPoint.y + ((halfRandMax - (double)rand()) / randMax) * amp * amp_y;
					newPoint.z = oldPoint.z + ((halfRandMax - (double)rand()) / randMax) * amp * amp_z;
					oldPointsArrays[0].append( oldPoint);
					newPointsArrays[0].append( newPoint);
				}
			}
		}
	}

	if(!selVtxMode)
	{
		num_meshes = res_MDagPathArray.length();
		newPointsArrays = new MPointArray[num_meshes];
		oldPointsArrays = new MPointArray[num_meshes];
		for( i = 0; i < num_meshes; i++ )
		{
			MDagPath dagPath = res_MDagPathArray[i];
			MObject node;

			node = dagPath.node();
			MFnDependencyNode fnNode( node );
			
			MStatus polyStatus;
			MFnMesh fnMesh( node, &polyStatus );

			if( polyStatus == MS::kSuccess )
			{
				MStatus status;
				status = fnMesh.getPoints( oldPointsArrays[i]);
				if (!status){status.perror("### error getting mesh points");return stat;}
				unsigned int numVertices = oldPointsArrays[i].length();
				newPointsArrays[i].setLength(numVertices);
				oldPointsArrays[i].setLength(numVertices);
				if(seed != 0) srand(seed);
				if(useAxisAmp)
				{
					for( unsigned int v = 0; v < numVertices; v++)
					{
						newPointsArrays[i][v].x = oldPointsArrays[i][v].x + ((halfRandMax - (double)rand()) / randMax) * amp * amp_x;
						newPointsArrays[i][v].y = oldPointsArrays[i][v].y + ((halfRandMax - (double)rand()) / randMax) * amp * amp_y;
						newPointsArrays[i][v].z = oldPointsArrays[i][v].z + ((halfRandMax - (double)rand()) / randMax) * amp * amp_z;
					}
				}
				else
				{
					for( unsigned int v = 0; v < numVertices; v++)
					{
						newPointsArrays[i][v].x = oldPointsArrays[i][v].x + ((halfRandMax - (double)rand()) / randMax) * amp;
						newPointsArrays[i][v].y = oldPointsArrays[i][v].y + ((halfRandMax - (double)rand()) / randMax) * amp;
						newPointsArrays[i][v].z = oldPointsArrays[i][v].z + ((halfRandMax - (double)rand()) / randMax) * amp;
					}
				}
			}
			else stat = MS::kFailure;
		}
	}
	return redoIt();
}

MStatus tm_randPoint::redoIt()
{
	if(helpMode) return MS::kSuccess;
	if(selVtxMode)
	{
		MSelectionList curSel;
		MGlobal::getActiveSelectionList( curSel);
		MItSelectionList iter( curSel, MFn::kMeshVertComponent);
		MObject component;
		MDagPath selVtx_dagPath;
		int vtxCount = 0;
		for ( ; !iter.isDone(); iter.next() )
		{
			iter.getDagPath(selVtx_dagPath, component);
			MItMeshVertex vertexIter( selVtx_dagPath, component );
			for ( ; !vertexIter.isDone(); vertexIter.next() )
			{
				vertexIter.setPosition( newPointsArrays[0][vtxCount], MSpace::kObject );
				vtxCount++;
			}
		}
	}
	else
	{
		int num_meshes = res_MDagPathArray.length();
		for( int i = 0; i < num_meshes; i++ )
		{
			MDagPath dagPath = res_MDagPathArray[i];
			MObject node;

			node = dagPath.node();
			MFnDependencyNode fnNode( node );
			
			MStatus polyStatus;
			MFnMesh fnMesh( node, &polyStatus );
			if( polyStatus == MS::kSuccess )
			{
				MStatus status;
				status = fnMesh.setPoints( newPointsArrays[i]);
				if (!status){status.perror("### error setting point ::redoIt()");return status;}
			}
		}
	}
	return MS::kSuccess;
}

MStatus tm_randPoint::undoIt()
{
	if(helpMode) return MS::kSuccess;
	if(selVtxMode)
	{
		MSelectionList curSel;
		MGlobal::getActiveSelectionList( curSel);
		MItSelectionList iter( curSel, MFn::kMeshVertComponent);
		MObject component;
		MDagPath selVtx_dagPath;
		int vtxCount = 0;
		for ( ; !iter.isDone(); iter.next() )
		{
			iter.getDagPath(selVtx_dagPath, component);
			MItMeshVertex vertexIter( selVtx_dagPath, component );
			for ( ; !vertexIter.isDone(); vertexIter.next() )
			{
				vertexIter.setPosition( oldPointsArrays[0][vtxCount], MSpace::kObject );
				vtxCount++;
			}
		}
	}
	else
	{
		int num_meshes = res_MDagPathArray.length();
		for( int i = 0; i < num_meshes; i++ )
		{
			MDagPath dagPath = res_MDagPathArray[i];
			MObject node;

			node = dagPath.node();
			MFnDependencyNode fnNode( node );
			
			MStatus polyStatus;
			MFnMesh fnMesh( node, &polyStatus );
			if( polyStatus == MS::kSuccess )
			{
				MStatus status;
				status = fnMesh.setPoints( oldPointsArrays[i]);
				if (!status){status.perror("### error setting point ::undoIt()");return status;}
			}
		}
	}
	return MS::kSuccess;
}

bool tm_randPoint::isUndoable() const
{
	return true;
}
