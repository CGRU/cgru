#define PI 3.1415926535897932384626433832795
#define PI4d5 2.5132741228718345907701147066236
#define PI2d3 2.0943951023931954923084289221863
#define PId2 1.5707963267948966192313216916398
#define PI2d5 1.2566370614359172953850573533118
#define PId3 1.0471975511965977461542144610932
#define PId4 0.78539816339744830961566084581988
#define PId5 0.6283185307179586476925286766559

#define McheckErr(stat,msg)         \
    if ( MS::kSuccess != stat ) {   \
        stat.perror(msg);                \
        return MS::kFailure;        \
    }

#include "geoSphere.h"

MTypeId tm_makeGeoSphere::id( tm_makeGeoSphere__id );

MObject tm_makeGeoSphere::primitive;
MObject tm_makeGeoSphere::frequency;
MObject tm_makeGeoSphere::radius;

MObject tm_makeGeoSphere::outMesh;

tm_makeGeoSphere::tm_makeGeoSphere(){}

tm_makeGeoSphere::~tm_makeGeoSphere() {}

void* tm_makeGeoSphere::creator(){	return new tm_makeGeoSphere();}

int tm_makeGeoSphere::AnzahlPunkte(int Freq)
{
	int i=Freq+1;
	int AnzPt=0;
	while( i > 0)
	{
		AnzPt = AnzPt + i;
		i = i - 1;
	}
	return AnzPt;
}

void tm_makeGeoSphere::rotMPointArray( MPointArray& rotFrom, MPointArray& rotTo, MEulerRotation& rotEuler)
{
	MMatrix rotMatrix = rotEuler.asMatrix();
	for ( int i = 0; i < segVtxCount;  i++)
		rotTo[i] = rotFrom[i] * rotMatrix;
}

MPoint tm_makeGeoSphere::getCenterPoint( MPoint& pointA, MPoint& pointB)
{
	MPoint res;
	double x = (pointA.x + pointB.x)/2;
	double y = (pointA.y + pointB.y)/2;
	double z = (pointA.z + pointB.z)/2;
	double l = sqrt( x*x + y*y + z*z);
	res.x = x/l;
	res.y = y/l;
	res.z = z/l;
	return res;
}

int tm_makeGeoSphere::getIndex( int w, int h, int freq)
{
	int index = 0;
	int fh = freq - h;
	for( int ih = freq; ih > fh; ih--)
		index += ih + 1;
	index += w;
	return index;
}

void tm_makeGeoSphere::getIndexWH( int& w, int& h, int index, int freq)
{
	h = 0;
	int i = 0;
	int ih;
	for( ih = freq; ih >= 0; ih--)
	{
		if((index - i - ih - 1) < 0) break;
		i += ih + 1;
		h++;
	}
	w = index - i;
}

void tm_makeGeoSphere::getUpperIndexesWH( int w, int h, int& wa, int& ha, int& wb, int& hb)
{
	if((w % 2) == 0)
	{
		wa = w >> 1;
		wb = w >> 1;
	}		
	else
	{
		wa = w >> 1;
		wb = (w >> 1) + 1;
	}
	if((h % 2) == 0)
	{
		ha = h >> 1;
		hb = h >> 1;
	}		
	else
	{
		ha = (h >> 1) + 1;
		hb = h >> 1;
	}
}

void tm_makeGeoSphere::getUpperIndexes( int index, int freq, int& w, int& h)
{
	int hi_w; int hi_h;
	getIndexWH( hi_w, hi_h, index, freq);
	int lo_wa, lo_ha, lo_wb, lo_hb;
	getUpperIndexesWH( hi_w, hi_h, lo_wa, lo_ha, lo_wb, lo_hb);
	w = getIndex( lo_wa, lo_ha, int(0.5 * freq));
	h = getIndex( lo_wb, lo_hb, int(0.5 * freq));
}

MStatus tm_makeGeoSphere::compute( const MPlug& plug, MDataBlock& data )
{    
    MStatus returnStatus;

	if (plug == outMesh)
	{
		MStatus stat;
        
		MDataHandle primitiveData = data.inputValue(primitive,&returnStatus);
		McheckErr(returnStatus, "Error getting radius primitive handle\n");
		int primitive = primitiveData.asInt();
        
		MDataHandle frequencyData = data.inputValue(frequency,&returnStatus);
		McheckErr(returnStatus, "Error getting frequency data handle\n");
		int frequency = frequencyData.asInt();

		MDataHandle radiusData = data.inputValue(radius,&returnStatus);
		McheckErr(returnStatus, "Error getting radius data handle\n");
		double radius = radiusData.asDouble();

		MDataHandle outputHandle = data.outputValue(outMesh, &returnStatus);
		McheckErr(returnStatus, "ERROR getting polygon data handle\n");
        MObject mesh = outputHandle.asMesh();
               
        MFnMeshData dataCreator;
        MObject newOutputData = dataCreator.create(&returnStatus);
        McheckErr(returnStatus, "ERROR creating outputData");

    	MIntArray		fec;
    	MDoubleArray	sp;
    	MDoubleArray	tp;

		int freq = frequency;
		frequency = 1 << (freq-1);
		MPointArray *segPoints = new MPointArray[freq];
		segPoints[0].setLength( 3);
// setting first triangle data
		switch (primitive)
		{
		case 1:
			segments = 4;
			segPoints[0][0] = MPoint( 0.9396926, -0.3420201, 0, 1);
			segPoints[0][1] = MPoint( -0.4714043, -0.3333331, 0.8164968, 1);
			segPoints[0][2] = MPoint( 0, 1, 0, 1);
			break;
		case 2:
			segments = 8;
			segPoints[0][0] = MPoint( 1, 0, 0, 1);
			segPoints[0][1] = MPoint( 0, 0, 1, 1);
			segPoints[0][2] = MPoint( 0, 1, 0, 1);
			break;
		case 3:
			segments = 20;
			segPoints[0][0] = MPoint( 0.8944271, 0.4472137, 0, 1);
			segPoints[0][1] = MPoint( 0.2763935, 0.4472137, 0.8506507, 1);
			segPoints[0][2] = MPoint( 0, 1, 0, 1);
			break;
		}

		int i, j, seg, index;
		segVtxCount  = AnzahlPunkte(frequency);
		segFaceCount = frequency;
		if( frequency > 1) segFaceCount += 2;
		for( i = frequency - 1 ; i > 1; i--)
			segFaceCount += (i<<1);
		int allVtxCount = segVtxCount * segments;
		int meshFaceCount = segFaceCount * segments;
		int faceVertices = meshFaceCount * 3;
//----------------------------- getting first segment vertices begin ----------------------------------
		{
			for( int f = 2; f <= freq; f++)
			{
				int fr = (1 << (f-1));
				segPoints[f - 1].setLength( AnzahlPunkte(fr));
				index = 0;
				for (i = fr; i >= 0; i--)
				{
					for (j = 0; j <= i; j++)
					{
						int lo_ia, lo_ib;
						getUpperIndexes( index, fr, lo_ia, lo_ib);
						MPoint centerPoint = getCenterPoint( segPoints[f-2][lo_ia], segPoints[f-2][lo_ib]);
						segPoints[f - 1][index] = centerPoint;
						index++;
					}
				}
			}
			for( int f = 0; f < freq; f++)
			{
				index = 0;
				int fr = (1 << f);
				for (i = fr; i >= 0; i--)
				{
					for (j = 0; j <= i; j++)
					{
						index++;
					}
				}
			}
		}
//---------------------------------------------------------------------- getting first segment vertices end

		MPointArray *pointsArray = new MPointArray[segments];
		for( seg = 0; seg < segments; seg++)
			pointsArray[seg].setLength(segVtxCount);

		for (index = 0; index < segVtxCount; index++)
				pointsArray[0][index] = segPoints[freq - 1][index] * radius;
		delete [] segPoints;

//------------------------ rotating vertices arrays begin ----------------------------------------------
      MEulerRotation eulerRotation;
      switch (primitive)
      {
      case 1:
         for ( seg = 1; seg < 3; seg++)
         {
            eulerRotation = MEulerRotation( 0, PI2d3 * seg, 0);
            rotMPointArray( pointsArray[0], pointsArray[seg], eulerRotation);
         }
         eulerRotation = MEulerRotation( 0, PId3, -1.9106332251750669186792027532919);//109.47122 /3= 36.4904067 /4= 27.367805
         rotMPointArray( pointsArray[0], pointsArray[3], eulerRotation);//109.47122 /3= 36.4904067 /4= 27.367805
         break;
      case 2:
         for( seg = 1; seg < 4; seg++)
         {
           eulerRotation = MEulerRotation( 0, PId2 * seg, 0);
           rotMPointArray( pointsArray[0], pointsArray[seg], eulerRotation);
         }
         for( seg = 4; seg < 8; seg++)
         {
           eulerRotation = MEulerRotation( PI, PId2 * (seg - 5), 0);
           rotMPointArray( pointsArray[0], pointsArray[seg], eulerRotation);
         }
         break;
      case 3:
         eulerRotation = MEulerRotation( 0, PId5, 0);
         rotMPointArray( pointsArray[0], pointsArray[0], eulerRotation);
         eulerRotation = MEulerRotation( 0, PI4d5, PI, MEulerRotation::kZYX);
         rotMPointArray( pointsArray[0], pointsArray[10], eulerRotation);
         j = 0;
         for (seg=5; seg<=9; seg++)
         {
            eulerRotation = MEulerRotation( 0, (PI2d5*j-PId5), -1.1071487177430917964236657694458, MEulerRotation::kZYX);
            rotMPointArray( pointsArray[0], pointsArray[seg], eulerRotation);
            j++;
         }
         j = 0;
         for (seg=15; seg<=19; seg++)
         {
            eulerRotation = MEulerRotation( 0, (PI2d5*j+PI), PI-1.1071487177430917964236657694458, MEulerRotation::kZYX);
            rotMPointArray( pointsArray[0], pointsArray[seg], eulerRotation);
            j++;
         }
         for (seg=1; seg<=4; seg++)
         {
           eulerRotation = MEulerRotation( 0, PI2d5*seg, 0);
           rotMPointArray( pointsArray[0], pointsArray[seg], eulerRotation);
         }
         for (seg=11; seg<=14; seg++)
         {
           eulerRotation = MEulerRotation( 0, PI2d5*seg + PI4d5, PI, MEulerRotation::kZYX);
           rotMPointArray( pointsArray[0], pointsArray[seg], eulerRotation);
         }
         break;
      }
//-------------------------------------------------------------------------- rotating vertices arrays end

		VertexStruct *verticesStruct = new  VertexStruct[allVtxCount];
		for( seg = 0; seg < segments; seg++)
			for( i = 0; i < segVtxCount;  i++)
			{
				verticesStruct[i + segVtxCount * seg].x = pointsArray[seg][i].x;
				verticesStruct[i + segVtxCount * seg].y = pointsArray[seg][i].y;
				verticesStruct[i + segVtxCount * seg].z = pointsArray[seg][i].z;
				verticesStruct[i + segVtxCount * seg].del = false;
				verticesStruct[i + segVtxCount * seg].pConCount = 0;
			}
		delete [] pointsArray;

    	MIntArray pCounts;
		pCounts.setLength( meshFaceCount);
		for( i = 0; i < meshFaceCount; i++)
			pCounts.set( 3, i);
			

//---------------------- searching for seem vertices begin -------------------------------------------
		int remCount = 0;
		int *remove;
		int *replace;
		int rem, remf, rep, repf;
		j = 0;
		switch (primitive)
		{
		case 1://---------------- searching for seem vertices for tetra ---------------------------
			remCount += (frequency + 1) * 2;			//	tetra seg[0..->..2]
			remCount += frequency;						//	tetra seg[2<-0]
			remCount += frequency*3;					//	tetra bot hat
			remove = new int[remCount];
			replace = new int[remCount];
			for( seg = 0; seg < 2; seg++)				//	tetra seg[0..->..2]
			{
				rem = frequency;
				remf = frequency;
				rep = 0;
				repf = frequency + 1;
				for(i = 0; i <= frequency; i++)
				{
					remove[j] = segVtxCount*(seg + 1) + rem;
					rem += remf;
					remf--;
					if(rep == (segVtxCount-1))	replace[j] = segVtxCount-1;
					else						replace[j] = segVtxCount*seg + rep;
					rep += repf;
					repf--;
					j++;
				}
			}
			rem = 0;									//	tetra seg[2<-0]
			remf = frequency + 1;
			rep = frequency;
			repf = frequency;
			for(i = 0; i < frequency; i++)
			{
				remove[j] = segVtxCount*2 + rem;
				rem += remf;
				remf--;
				replace[j] = rep;
				rep += repf;
				repf--;
				j++;
			}
			rem = 0;									//	tetra seg[3->2]
			remf = 1;
			rep = frequency;
			repf = 1;
			for(i = 0; i < frequency; i++)
			{
				remove[j] = segVtxCount*3 + rem;
				rem += remf;
				if(rep == frequency)
                	replace[j] = segVtxCount;
				else
                	replace[j] = segVtxCount*2 + rep;
				rep -= repf;
				j++;
			}
			rem = frequency + 1;							//	tetra seg[3->1]
			remf = frequency;
			rep = 1;
			repf = 1;
			for(i = 0; i < frequency; i++)
			{
				remove[j] = segVtxCount*3 + rem;
				rem += remf;
				remf--;
				if(rep == frequency)
                	replace[j] = 0;
				else
                	replace[j] = segVtxCount + rep;
				rep += repf;
				j++;
			}
			rem = frequency;							//	tetra seg[3->0]
			remf = frequency;
			rep = frequency;
			repf = 1;
			for(i = 0; i < frequency; i++)
			{
				remove[j] = segVtxCount*3 + rem;
				rem += remf;
				remf--;
                	replace[j] = rep;
				rep -= repf;
				j++;
			}
			break;
//-------------------------- searching for seem vertices for octa ----------------------------------
		case 2:
			remCount += (frequency + 1) * 3;			//	octa seg[0..->..3]
			remCount += frequency;						//	octa seg[3<-0]
			remCount += frequency * 3;					//	octa seg[4..->..7]
			remCount += frequency - 1;						//	octa seg[7<-4]
			remCount += (frequency+1)*4;					//	octa seg[4..->0..]
			remove = new int[remCount];
			replace = new int[remCount];
			for( seg = 0; seg < 3; seg++)				//	octa seg[0..->..3]
			{
				rem = frequency;
				remf = frequency;
				rep = 0;
				repf = frequency + 1;
				for(i = 0; i <= frequency; i++)
				{
					remove[j] = segVtxCount*(seg + 1) + rem;
					if(rep == (segVtxCount-1))	replace[j] = segVtxCount-1;
					else						replace[j] = segVtxCount*seg + rep;
					rem += remf;
					remf--;
					rep += repf;
					repf--;
					j++;
				}
			}
			rem = 0;									//	octa seg[3<-0]
			remf = frequency + 1;
			rep = frequency;
			repf = frequency;
			for(i = 0; i < frequency; i++)
			{
				remove[j] = segVtxCount*3 + rem;
				replace[j] = rep;
				rem += remf;
				remf--;
				rep += repf;
				repf--;
				j++;
			}
			for( seg = 4; seg < 7; seg++)				//	octa seg[4..->..7]
			{
				rem = frequency + 1;
				remf = frequency;
				rep = frequency * 2;
				repf = frequency - 1;
				for(i = 0; i < frequency; i++)
				{
					remove[j] = segVtxCount*(seg + 1) + rem;
					if(rep == (segVtxCount-1))	replace[j] = segVtxCount*5-1;
					else						replace[j] = segVtxCount*seg + rep;
					rem += remf;
					remf--;
					rep += repf;
					repf--;
					j++;
				}
			}
			rem = frequency*2;									//	octa seg[7<-4]
			remf = frequency-1;
			rep = frequency + 1;
			repf = frequency;
			for(i = 1; i < frequency; i++)
			{
				remove[j] = segVtxCount*7 + rem;
				replace[j] = segVtxCount*4 + rep;
				rem += remf;
				remf--;
				rep += repf;
				repf--;
				j++;
			}
			for( seg = 0; seg < 4; seg++)					//	octa seg[4..->0..]
			{
				rem = 0;
				remf = 1;
				rep = frequency;
				repf = 1;
				for(i = 0; i <= frequency; i++)
				{
					remove[j] = segVtxCount*(seg + 4) + rem;
					if((rep == 0)&&(seg == 3))
						replace[j] = frequency;
					else
					{
						if((rep == frequency)&&(seg != 0)) replace[j] = segVtxCount*(seg - 1);
						else replace[j] = segVtxCount*seg + rep;
					}
					rem += remf;
					rep -= repf;
					j++;
				}
			}
			break;
//-------------------------- searching for seem vertices for icosa ----------------------------------
		case 3:
			remCount += (frequency + 1) * 4;			//	icosa seg[0..->..4]
			remCount += frequency;						//	icosa seg[4<-0]
			remCount += (frequency + 1) * 4;			//	icosa seg[10..<-..14]
			remCount += frequency;						//	icosa seg[14->10]
			remCount += (frequency-1)*5;				//	icosa seg[15->5..19->9]
			remCount += (frequency-1)*5;				//	icosa seg[15->6..19->5]
			remCount += (frequency+1)*5;				//	icosa top hat
			remCount += (frequency+1)*5;				//	icosa bot hat
			remCount += 5;								//	icosa top holes
			remCount += 5;								//	icosa bot holes
			remove = new int[remCount];
			replace = new int[remCount];
			for( seg = 0; seg < 4; seg++)				//	icosa seg[0..->..4]
			{
				rem = frequency;
				remf = frequency;
				rep = 0;
				repf = frequency + 1;
				for(i = 0; i <= frequency; i++)
				{
					remove[j] = segVtxCount*(seg + 1) + rem;
					if(rep == (segVtxCount-1))	replace[j] = segVtxCount-1;
					else						replace[j] = segVtxCount*seg + rep;
					rem += remf;
					remf--;
					rep += repf;
					repf--;
					j++;
				}
			}
			rem = 0;									//	icosa seg[4<-0]
			remf = frequency + 1;
			rep = frequency;
			repf = frequency;
			for(i = 0; i < frequency; i++)
			{
				remove[j] = segVtxCount*4 + rem;
				replace[j] = rep;
				rem += remf;
				remf--;
				rep += repf;
				repf--;
				j++;
			}
			for( seg = 10; seg < 14; seg++)				//	icosa seg[10..<-..14]
			{
				rem = 0;
				remf = frequency + 1;
				rep = frequency;
				repf = frequency;
				for(i = 0; i <= frequency; i++)
				{
					remove[j] = segVtxCount*(seg + 1) + rem;
					if(rep == (segVtxCount-1))	replace[j] = segVtxCount*10 + segVtxCount-1;
					else						replace[j] = segVtxCount*seg + rep;
					rem += remf;
					remf--;
					rep += repf;
					repf--;
					j++;
				}
			}
			rem = frequency;									//	icosa seg[14->10]
			remf = frequency;
			rep = 0;
			repf = frequency + 1;
			for(i = 0; i < frequency; i++)
			{
				remove[j] = segVtxCount*14 + rem;
				replace[j] = segVtxCount*10 + rep;
				rem += remf;
				remf--;
				rep += repf;
				repf--;
				j++;
			}
			for(seg = 15; seg < 20; seg++)						//	icosa seg[15..->..5]
			{
				rem = frequency + 1;
				remf = frequency;
				rep = segVtxCount - 3;
				repf = 3;
				for(i = 1; i < frequency; i++)
				{
					remove[j] = segVtxCount*seg + rem;
					replace[j] = segVtxCount*(seg-10) + rep;
					rem += remf;
					remf--;
					rep -= repf;
					repf++;
					j++;
				}
			}
			for(seg = 15; seg < 20; seg++)						//	icosa seg[15..->..6]
			{
				rem = frequency*2;
				remf = frequency-1;
				rep = segVtxCount - 2;
				repf = 2;
				for(i = 1; i < frequency; i++)
				{
					remove[j] = segVtxCount*seg + rem;
					if(seg == 19)
						replace[j] = segVtxCount*5 + rep;
					else
						replace[j] = segVtxCount*(seg - 9) + rep;
					rem += remf;
					remf--;
					rep -= repf;
					repf++;
					j++;
				}
			}
			for(seg = 15; seg < 20; seg++)					//	icosa top hat
			{
				rem = 0;
				remf = 1;
				rep = frequency;
				repf = 1;
				for(i = 0; i <= frequency; i++)
				{
					remove[j] = segVtxCount*seg + rem;
					if(rem == 0)
					{
						if(seg == 15) replace[j] = frequency;
						else replace[j] = segVtxCount*(seg - 16);
					}
					else if ((rem == frequency)&&(seg == 19))
                        replace[j] = frequency;
					else replace[j] = segVtxCount*(seg - 15) + rep;
					rem += remf;
					rep -= repf;
					j++;
				}
			}
			for(seg = 5; seg < 10; seg++)					//	icosa bot hat
			{
				rem = frequency;
				remf = 1;
				rep = 0;
				repf = 1;
				for(i = 0; i <= frequency; i++)
				{
					remove[j] = segVtxCount*seg + rem;
					if(rem == frequency)
					{
						if(seg == 5) replace[j] = segVtxCount*10;
						else replace[j] = segVtxCount*(seg + 4) + frequency;
					}
					else
					{
						if((rem == 0)&&(seg == 9)) replace[j] = segVtxCount*10;
                        else replace[j] = segVtxCount*(seg + 5) + rep;
					}
					rem -= remf;
					rep += repf;
					j++;
				}
			}
			remove[j] = segVtxCount*5 + segVtxCount-1;
			replace[j] = frequency;
			j++;
			for(seg = 6; seg < 10; seg++)
			{
				remove[j] = segVtxCount*seg + segVtxCount-1;
				replace[j] = segVtxCount*(seg - 6);
				j++;
			}
			for(seg = 15; seg < 19; seg++)
			{
				remove[j] = segVtxCount*seg + segVtxCount-1;
				replace[j] = segVtxCount*(seg - 5) + frequency;
				j++;
			}
			remove[j] = segVtxCount*19 + segVtxCount-1;
			replace[j] = segVtxCount*10;
			break;
		}
//------------------------------------------------------------------ searching for seem vertices end --------
		for( rem = 0; rem < remCount; rem++)
		{
			verticesStruct[remove[rem]].del = true;
			verticesStruct[remove[rem]].rep = replace[rem];
		}
	//-------------------------------- making pConnect begin ------------------------------------------------
		{
			int f = frequency;
			int faceId = 0;
			int p = 0;
			int q = 1;
			int lp= f - 1;
			while (f > 0)
			{
				while (p < lp+1)
				{
					for( seg = 0; seg < segments; seg++)
					{
						verticesStruct[segVtxCount * seg + p+1  ].pConArray[verticesStruct[segVtxCount * seg + p+1  ].pConCount++] = 3 * (segFaceCount * seg + faceId);
						verticesStruct[segVtxCount * seg + p    ].pConArray[verticesStruct[segVtxCount * seg + p    ].pConCount++] = 3 * (segFaceCount * seg + faceId) + 1;
						verticesStruct[segVtxCount * seg + p+f+1].pConArray[verticesStruct[segVtxCount * seg + p+f+1].pConCount++] = 3 * (segFaceCount * seg + faceId) + 2;
					}
					faceId++;
					p += 1;
				}
				p += 1;
				if (f > 1)
				{
					while (q < lp + 1)
					{
						for( seg = 0; seg < segments; seg++)
						{
						verticesStruct[segVtxCount * seg + q    ].pConArray[verticesStruct[segVtxCount * seg + q    ].pConCount++] = 3 * (segFaceCount * seg + faceId);
						verticesStruct[segVtxCount * seg + q+f  ].pConArray[verticesStruct[segVtxCount * seg + q+f  ].pConCount++] = 3 * (segFaceCount * seg + faceId) + 1;
						verticesStruct[segVtxCount * seg + q+f+1].pConArray[verticesStruct[segVtxCount * seg + q+f+1].pConCount++] = 3 * (segFaceCount * seg + faceId) + 2;
						}
						faceId++;
						q += 1;
					}
					q += 2;
				}
				lp = lp + f;
				f = f - 1;
			}
		}
//-------------------------------------------------------------------------- making pConnect end
		int mesh_verticesCount = allVtxCount - remCount;
		MPointArray mesh_vertices;
		mesh_vertices.setLength( mesh_verticesCount);
		MIntArray mesh_pConnect;
		mesh_pConnect.setLength( faceVertices);
		int *pcToRep = new int[ faceVertices];
		rep = 0;
		index = 0;
		for( i = 0; i < allVtxCount; i++)
		{

			if(verticesStruct[i].del)
			{
				for( j = 0; j < verticesStruct[i].pConCount; j++)
				{
					mesh_pConnect.set( verticesStruct[i].rep, verticesStruct[i].pConArray[j]);
					pcToRep[rep++] = verticesStruct[i].pConArray[j];
				}	
				continue;
			}
			else
			{
				for( j = 0; j < verticesStruct[i].pConCount; j++)
				{
					mesh_pConnect.set( index, verticesStruct[i].pConArray[j]);
					pcToRep[verticesStruct[i].pConArray[j]] = false;
				}
			}
			mesh_vertices[index].x = verticesStruct[i].x;
			mesh_vertices[index].y = verticesStruct[i].y;
			mesh_vertices[index].z = verticesStruct[i].z;
			verticesStruct[i].newIndex = index;
			index++;
		}
		for( i = 0; i < rep; i++)
		{
			int oldCon = mesh_pConnect[pcToRep[i]];
			int newCon = verticesStruct[oldCon].newIndex;
			mesh_pConnect.set( newCon, pcToRep[i]);
		}
		delete [] verticesStruct;
		delete [] replace;
		delete [] remove;
		delete [] pcToRep;
//----------------- creating mesh from arrays -------------------------------------------
		MFnMesh meshFn;
    	mesh= meshFn.create(
		    	mesh_verticesCount,
		    	meshFaceCount,
		    	mesh_vertices,
		    	pCounts,
		    	mesh_pConnect,
		    	newOutputData,
		    	&returnStatus
  	    );
		outputHandle.set(newOutputData);
        data.setClean( plug );
	}
	return MS::kSuccess;
}

MStatus tm_makeGeoSphere::initialize()
{
    MFnTypedAttribute   typedFn;
	MStatus			    stat;

    outMesh = typedFn.create( "outMesh", "o", MFnData::kMesh, &stat ); 
    if ( MS::kSuccess != stat ) {
    	stat.perror("ERROR creating tm_makeGeoSphere output attribute");
        return stat;
    }
    typedFn.setStorable(false);
    typedFn.setWritable(false);
    stat = addAttribute( outMesh );
    McheckErr(stat, "ERROR adding attribute");	
   
	MFnEnumAttribute eAttr;

	primitive = eAttr.create( "primitive", "p", 3, &stat );
	stat = eAttr.addField( "tetra", 1);
	stat = eAttr.addField( "octa", 2);
	stat = eAttr.addField( "icosa", 3);
	stat = eAttr.setKeyable  ( true );
	stat = eAttr.setCached   ( true );
	stat = eAttr.setStorable ( true );
	stat = addAttribute( primitive );
	stat = attributeAffects( primitive, outMesh );
    McheckErr(stat, "ERROR adding primitive attribute");

	MFnNumericAttribute nAttr;

	frequency = nAttr.create( "frequency", "f", MFnNumericData::kInt, 3, &stat );
	stat = nAttr.setMin(1);
	stat = nAttr.setMax(6);
	stat = nAttr.setKeyable  ( true );
	stat = nAttr.setCached   ( true );
	stat = nAttr.setStorable ( true );
	stat = addAttribute( frequency );
	stat = attributeAffects( frequency, outMesh );
    McheckErr(stat, "ERROR adding frequency attribute");

	radius = nAttr.create( "radius", "r", MFnNumericData::kDouble, 1.0, &stat );
	stat = nAttr.setMin(0);
	stat = nAttr.setMax(1000);
	stat = nAttr.setKeyable  ( true );
	stat = nAttr.setCached   ( true );
	stat = nAttr.setStorable ( true );
	stat = addAttribute( radius );
	stat = attributeAffects( radius, outMesh );
   McheckErr(stat, "ERROR adding radius attribute");


	return MS::kSuccess;
}
