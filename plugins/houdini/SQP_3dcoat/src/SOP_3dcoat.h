/// \file SOP_3dcoat.h
/// Definition of 3dCoat reader node.
///
/// \author nazarenko.sergiy@gmail.com
///

#ifndef __SOP_3DCOAT_h__
#define __SOP_3DCOAT_h__

#include <SOP/SOP_Node.h>
typedef unsigned char		u8;

const char* FILE_NAME_PARM =       "file_name";
const char* RELOAD_GEOMETRY_PARM = "reload_geometry";


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//Definition custom data types
typedef unsigned char byte; 
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long qword;
typedef dword FsType;


/// Class for reading 3B files.
/// This class is really doing one thing:
/// reads 3B file and provides some functions to get neccessary data
/// Code was borrowed from http://www.3d-coat.com/forum/index.php?showtopic=3227 . 3B file format specification places as well.
/// Code do over for OS Linux.
class ReadBinStream
{
protected:
    
    FsType pos;    ///< Position in 3d file.
    FsType fSize;  ///< Size of file.

    char* tempBuffer;
    FsType tempSize;
    FsType tempPos;
    FsType tempCurrSize;
    
    //Defining data that filled by ReadData() and returned by functions
    int sizeOfCell;
    float** all_values; ///< Array of volume values
    int** resOfValues; ///< Array of resolution volume values
    char** all_names; ///< Array of volume names
    float** all_matrix; ///< Array of transform matrix
    int numVolumes; ///< Number of volumes
    
    void ReadData(); ///< Reads all data from file.
    
    /// Values are stored with RLE compression.
    /// There is algorithm for decompression (c++). Refer to 3B file format specification.
    /// \code
    /// RestoreRLE2(&vox[0],&fvox[0], 729);
    /// \endcode
    /// \param wDst array of word 
    /// \param fDst array of float 
    /// \param size size of array.
    void RestoreRLE2(word* , float *, int );
    
    
    byte  ReadBYTE();
    word ReadWORD();
    dword ReadDWORD();
    
    ///Reading the file
    /// \param data pointer to data
    /// \param size size of data
    FsType Read(void* ,FsType);
    FsType GetReadPos(); ///< Size of file
    FsType Size(); ///< Size of file
    char * ReadString(); ///< Reads string data
    FsType Skip(FsType); ///< Skips block 
    
public:     
    /// Creates a new ReadBinStream object.
    /// \param Name file path to initialize class.
    ReadBinStream(const char* Name=NULL);
    
    /// Release memory. Delete all arrays
    ~ReadBinStream();
    
    /// \name Gets all neccesary data
    /// @{
    int GetSizeOfCell(); ///< Gets size of cell
    float** GetAllValues();///< Gets size of cell
    char** GetAllNames(); ///< Gets names of volumes
    int GetNumVolumes(); ///< Gets number of volumes
    int** GetResOfValues();///< Gets resolution of volumes
    float** GetAllMatrix();///< Gets transform matrix of volumes
};

/// Houdini node defenition
class SOP_3dcoat : public SOP_Node
{
public:
	SOP_3dcoat(OP_Network *net, const char *name, OP_Operator *op);
	virtual ~SOP_3dcoat();

	static PRM_Template     myTemplateList[];
	static OP_Node          *myConstructor(OP_Network*, const char *, OP_Operator *);

protected:
    
        bool mCalledFromCallback;
        
	virtual unsigned		 disableParms();
        
	virtual OP_ERROR		 cookMySop(OP_Context &context);
        
        //!Callback function to handle pushbutton in GUI
        static int
        reloadFile(void *data, int index, float time, const PRM_Template *tplate );
        
        //!Function for GUI widgets
        void FILENAME(UT_String &label) {     evalString(label, FILE_NAME_PARM, 0, 0);   }
};



#endif


