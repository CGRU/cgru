/// \file SOP_3dcoat.c
/// Implementation of 3dCoat reader node.
///
/// \author nazarenko.sergiy@gmail.com
///

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Math.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <GEO/GEO_AttributeHandle.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Director.h>
#include <SOP/SOP_Error.h>
#include <GU/GU_PrimVolume.h>
#include <GU/GU_SDF.h>
#include "SOP_3dcoat.h"

#include <cstring>

using namespace std;

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

ifstream::pos_type file_size;
char * memblock;


//Read MagicSignature 
inline dword MakeMagic(const char *Magic)
{
    int l = strlen(Magic);
    int i;
    dword c, M = 0;
    for(i = l - 1; i >= 0; i--) {
        c = Magic[i];
        M += c << (8 * (3 - i));
    }
    return M;
}

//////////////////////////////////////////////////////
ReadBinStream::~ReadBinStream()
{
     if(tempBuffer) delete tempBuffer;
     if(all_values) delete all_values;
     if(resOfValues) delete resOfValues;
     if(all_names) delete all_names;
     if(all_matrix) delete all_matrix;
}

ReadBinStream::ReadBinStream(const char* name)
{
    fSize=0;
    pos=0;
    sizeOfCell=0;
    tempSize=65536*128;
    tempBuffer=new char[tempSize];
    tempCurrSize=0;
    tempPos=0;
    numVolumes=0;
    
    ifstream file (name, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        fSize = file.tellg();
        tempSize = fSize;
        tempBuffer = new char [fSize];
        file.seekg (0, ios::beg);
        file.read (tempBuffer, fSize);
        file.close();
        ReadData();
    }
    else
    {
          cout << "The file not loaded " << endl;
    }
}

float** ReadBinStream::GetAllValues()
{
    return all_values;
}

char* ReadBinStream::ReadString()
{
	int L= ReadDWORD();
	char* s=new char[L];
	Read(s,L);
	return s;
}

int ReadBinStream::GetSizeOfCell()
{
    return sizeOfCell;
}

FsType ReadBinStream::Skip(FsType size)
{	
	tempPos+=size;
	pos+=size;
	return size;
}

FsType ReadBinStream::Read(void* data,FsType size)
{	
	byte* _data=(byte*)data;
	int ads=0;
        
        memcpy(_data,tempBuffer+tempPos,size);

        tempPos+=size;
	pos+=size;
        return size+ads;
}

byte ReadBinStream::ReadBYTE()
{
        byte b=0;
	Read(&b,1);
	return b;
}

word ReadBinStream::ReadWORD()
{
	word w=0;
	Read(&w,2);
	return w;
}

dword ReadBinStream::ReadDWORD()
{
	dword D=0;
	Read(&D,4);
	return D;
}

FsType ReadBinStream::Size()
{
	return fSize;
}

FsType ReadBinStream::GetReadPos()
{
	return pos;
}

char** ReadBinStream::GetAllNames()
{
    return all_names;
}

/// \todo Remove reading word data. Is not need more
void ReadBinStream::RestoreRLE2(word* wDst, float *fDst, int size)
{
    int pos=1;
    word V0= ReadWORD();
    wDst[0]=V0;
    fDst[0]=V0/65535;
    if(size)
    {
        do
        {
            byte sz= ReadBYTE();
            // There is convert cell data (0-65535) into houdini volume data (0-1)
            // Just cell data devide on 65535
            if(sz>=220)
            {
                sz=sz-220;
                for(int i=0;i<sz;i++)
                {
                    wDst[pos]=wDst[pos-1]+ ReadWORD();
                    fDst[pos]=(float)wDst[pos]/65535;
                    pos++;
                }
            }
            else
            {			
                for(int i=0;i<sz;i++)
                {
                    wDst[pos]=wDst[pos-1];
                    fDst[pos]=(float)wDst[pos]/65535;
                    pos++;
                }
            }
        }
        while(pos<size);
    }		
}

float** ReadBinStream::GetAllMatrix()
{
     return all_matrix;
}

int** ReadBinStream::GetResOfValues()
{
    return resOfValues;
}

int ReadBinStream::GetNumVolumes()
{
    return numVolumes;
}

void ReadBinStream::ReadData()
{
    
    float** exp_array;
    int** lresOfValues;
    char** vol_names;
    float** vol_matrix;
    
    dword M = ReadDWORD();
    if(M==MakeMagic("MESH"))
    {
        M = ReadDWORD();
	if(M == 1)
        {
            do
            {
                M=ReadDWORD();
                int sz=ReadDWORD();
                if(M==MakeMagic("VOL3"))
                {
                    int ver = ReadDWORD();
                    if(ver==5)
                    {
                        int nvolumes= ReadDWORD(); // Read number of volumes
			numVolumes = nvolumes; // Set global variable
                        
                        // Create arrays for global arrays
                        exp_array = new float * [nvolumes];
                        lresOfValues = new int * [nvolumes];
                        vol_names = new char * [nvolumes];
                        vol_matrix = new float * [nvolumes];
                        
                        
                        for(int j=0;j<nvolumes;j++)
                        {
                            
                            int maxX = 0;
                            int maxY = 0;
                            int maxZ = 0;
                            int minX = 0;
                            int minY = 0;
                            int minZ = 0;
                            
                            int SpaceID = ReadDWORD();
                            float* M = new float[16];
                            Read(M,16*4);
                            vol_matrix[j] = M;// Volume transform, 4x4 matrix
                            char* Name = ReadString();
                            vol_names[j] = Name;// Name of Volume
                            dword DefColor = ReadDWORD();
                            dword Repr = ReadDWORD();
                            /// \todo If Volume surface representation should fill geometry array ...
                            
                            //Representation – 256 if in surface representation, 0 if in voxel representation.
                         
                            int HidID= ReadDWORD();
                            char* s= ReadString(); // Reads shader name
                            delete[]s;
                            int nc= ReadDWORD(); // Reads number of cells
                            ReadDWORD();//skipping dword
                            float* vol_array = new float [9*9*9*nc];//Create cells array. One cell 9*9*9 multiply on number of cells
                            short* coord_array_array = new short [nc*3];
                            int h = 0;
                            int m = 0;
                            for(int i=0;i<nc;i++)
                            {
                                short x= ReadWORD();
                                short y= ReadWORD();
                                short z= ReadWORD();
                                
                                sizeOfCell= ReadBYTE();

                                coord_array_array[h] = x;
                                h++;
                                coord_array_array[h] = y;
                                h++;
                                coord_array_array[h] = z;
                                h++;
                                
                                // Calculate max and min coordinates
                                if (x < 0 and minX > x) minX = x;
                                if (y < 0 and minY > y) minY = y;
                                if (z < 0 and minZ > z) minZ = z;
                                if (x > 0 and maxX < x) maxX = x;
                                if (y > 0 and maxY < y) maxY = y;
                                if (z > 0 and maxZ < z) maxZ = z;
                                                                
                                int ms= ReadBYTE();
                                
                                int dflag= ReadBYTE();
                                int defv= ReadWORD();
                                if(dflag)
                                {
                                    word vox[729];
                                    float fvox[729];
                                    
                                    //Decompress data
                                    RestoreRLE2(&vox[0],&fvox[0],729);
                              
                                    for(int n=0;n<729;n++,m++) vol_array[m] = fvox[n];
                                }
                                else
                                {
                                    for(int n=0;n<729;n++,m++) vol_array[m] = (float)defv/65535;
                                }
                                //Has surface faces?
                                if(ms&4)
                                {
                                    int nv= ReadDWORD();
                                     Skip(nv*6*4);//skip vertices, each vertex is 24 bytes
                                    int ni= ReadDWORD();
                                     Skip(ni*2);
                                    int nv0= ReadDWORD();
                                     Skip(nv0*6*4);//skip vertices, each vertex is 24 bytes
                                }
                                
                            }
                            // Actual position of the cell in space might be positive and negative
                            // For C++ array should be always positive
                            // Makes array where indeces always positive and fill it
                            int ex_maxX = maxX + abs(minX) + 1;
                            int ex_maxY = maxY + abs(minY) + 1;
                            int ex_maxZ = maxZ + abs(minZ) + 1;

                            float* export_array = new float [9*9*9*ex_maxX*ex_maxY*ex_maxZ];
                            
                            for (int h=0, m=0;h<nc*3; h+=3)
                            {
				//Gets global coordinates of cell
                                int pX = (coord_array_array[h] + abs(minX));
                                int pY = (coord_array_array[h+1] + abs(minY));
                                int pZ = (coord_array_array[h+2] + abs(minZ));
				//Gets values and set global array
                                //Uses flat array - export_array[x+y*len(x)+z*len(x)*len(y)]
                                //where ingridients of global index is index in cell plus cell coordinate multiply on cell size
				for (int z1 = 0; z1 < 9; z1++)
				    for (int y1 = 0; y1 < 9; y1++)
					for (int x1 = 0; x1 < 9; x1++)
					{
					    export_array[((x1+pX*9) + (y1+pY*9)*ex_maxX*9 + (z1+pZ*9)*ex_maxX*9*ex_maxY*9)] = vol_array[m+x1+y1*9+z1*9*9];  
					}
				m+=729;
		            
                            }
			    
                            exp_array[j] = export_array;
                            int* asize = new int[3];
                            asize[0] =  ex_maxX;
                            asize[1] =  ex_maxY;
                            asize[2] =  ex_maxZ;
                            lresOfValues[j] = asize;
                            
                            delete vol_array;
                        }
                       
                    }
                    break;
                }
                else
                {
                    // If file begin XML data, skip it
                    Skip(sz);
                }
            } while ( GetReadPos()< Size());
        }
    }
    
    resOfValues = lresOfValues;
    all_names = vol_names;
    all_matrix = vol_matrix;
    all_values = exp_array;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/// Default file name for load 3b file
static PRM_Default nameDefault_filename(0,"$HIP/mySphere.3b");

static PRM_Name names[] =
{
    PRM_Name(FILE_NAME_PARM,       "File Name"),
    PRM_Name(RELOAD_GEOMETRY_PARM, "Reload File"),
    PRM_Name(0)
};

PRM_Template
SOP_3dcoat::myTemplateList[] =
{
    // Filename of the file to be exported
    PRM_Template(PRM_FILE, 1, &names[0], &nameDefault_filename, 0),
    // Reload Geometry button
    PRM_Template(PRM_CALLBACK, 1, &names[1], 0, 0, 0, SOP_3dcoat::reloadFile),
    PRM_Template()
};


///Reload geometry. Should only be active in read mode.
int
SOP_3dcoat::reloadFile(void *data, int index, float time, const PRM_Template *tplate)
{
    bool retval = 1;

    // Set state to indicate that we are being called via pushbutton
    SOP_3dcoat *me = (SOP_3dcoat *) data;
    me->mCalledFromCallback = true;

    // Enter the main cook routine
    OP_Context myContext(time);
    myContext.setData((OP_ContextData*)data);
    me->cookMe(myContext);

    return retval;
}

void
newSopOperator(OP_OperatorTable *table)
{
     table->addOperator(new OP_Operator("3dCoat",
					"3DCoat",
					 SOP_3dcoat::myConstructor,
					 SOP_3dcoat::myTemplateList,
					 0,
					 0,
					 0));
}


OP_Node *
SOP_3dcoat::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_3dcoat(net, name, op);
}

SOP_3dcoat::SOP_3dcoat(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op),mCalledFromCallback(false) {}

SOP_3dcoat::~SOP_3dcoat() {}

unsigned
SOP_3dcoat::disableParms()
{
    unsigned changed = 0;

    return changed;
}

//!Cook geometry for the SOP
OP_ERROR
SOP_3dcoat::cookMySop(OP_Context &context)
{
     UT_String		 fileName;
     
     // Clean out all geometry
     gdp->clearAndDestroy();
     
     FILENAME(fileName);
     
     if(!fileName.isstring())
     {
          cout << "File not found!" << endl;
          return error();
     }
     
     GB_PointGroup	*point_group = 0;
     
     // Read 3b file
     ReadBinStream *RBS = new ReadBinStream(fileName.toStdString().c_str());
     
     
     // Gets all data
     int GRP_count = RBS->GetNumVolumes();
     
     if (GRP_count == 0)
     {
          delete RBS;
          return error();
     }
     int res_cell = RBS->GetSizeOfCell();
     float** all_values = RBS->GetAllValues();
     int** res_val = RBS->GetResOfValues();
     char** all_names = RBS->GetAllNames();
     float** all_matrix = RBS->GetAllMatrix();
     
     
     // Create and fill GU_PrimVolumes.
     // Each volume will placed in own group
     for (int g_index = 0; g_index < GRP_count; g_index++)
     {
         
          GU_PrimVolume       *volume;
          
          const int xres = res_val[g_index][0]*res_cell;
          const int yres = res_val[g_index][1]*res_cell;
          const int zres = res_val[g_index][2]*res_cell;
          int x, y, z;
          
          int ii = 0;
          volume = (GU_PrimVolume *)GU_PrimVolume::build(gdp);
          // The COW handle will write data to the voxel array on destruction
          {
             UT_VoxelArrayWriteHandleF handle = volume->getVoxelWriteHandle();
          
             handle->size(xres, yres, zres);
             for (z = 0; z < zres; z++)
             {
                 for (y = 0; y < yres; y++)
                 {
                     for (x = 0; x < xres; x++)
                     {
                         handle->setValue(x, y, z, all_values[g_index][ii]);
                         ii++;
                     }
                 }
             }
          }
          
          /// \todo Transform matrix should be right apply to volume...    
          UT_Matrix3              xform;
          xform.identity();
          xform.scale(res_val[g_index][0]/10, res_val[g_index][1]/10, res_val[g_index][2]/10);
          volume->setTransform(xform);
          const UT_Matrix4 tm((float)all_matrix[g_index][0],
                              (float)all_matrix[g_index][1],
                              (float)all_matrix[g_index][2],
                              (float)all_matrix[g_index][3],
                              (float)all_matrix[g_index][4],
                              (float)all_matrix[g_index][5],
                              (float)all_matrix[g_index][6],
                              (float)all_matrix[g_index][7],
                              (float)all_matrix[g_index][8],
                              (float)all_matrix[g_index][9],
                              (float)all_matrix[g_index][10],
                              (float)all_matrix[g_index][11],
                              (float)all_matrix[g_index][12],
                              (float)all_matrix[g_index][13],
                              (float)all_matrix[g_index][14],
                              (float)all_matrix[g_index][15]);
          volume->transform(tm);
          
          // Create groups and call them as Volume names
          point_group = gdp->newPointGroup(all_names[g_index], false);
          point_group->add(gdp->points()[g_index]);
          
     }
     // After create GU_PrimVolume RBS class is not need - remove it
     delete RBS;
     return error();
}


