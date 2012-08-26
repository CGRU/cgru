#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfInputFile.h>
#include <ImfMatrixAttribute.h>
#include <ImfOutputFile.h>
#include <ImfStringAttribute.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILENAME_MAXLEN 4096

#ifdef WINNT
#define unlink _unlink
#define strncpy strncpy_s
#endif

// Forward declarations:
int joinEXRs( int, int, const char*, bool , bool);

int main ( int argc, char **argv )
{
   if( argc < 4 )
   {
      printf("Error: Not enough arguments provided.\n");
      printf("Usage: exrjoin tilesX tilesY filename.\n");
      printf("Example: exrjoin 2 3 img.exr [d]\n");
      printf("   Folder should contain 6 exr images: img.exr.tile_0.exr .. img.exr.tile_6.exr.\n");
      printf("   Last 'd' argument - to delete tiles if join was successful.\n");
      printf("   Any other additional arguments - program verbose mode.\n");
      return 1;
   }
   int tilesX = atoi( argv[1]);
   int tilesY = atoi( argv[2]);
   const char * name = argv[3];
   bool verbose = false;
   bool deleteTiles = false;
   if( argc >= 5 )
   {
      if( argv[4][0] == 'd')
      {
         deleteTiles = true;
         if( argc >=6 ) verbose = true;
      }
      else verbose = true;
   }
   if((tilesX > 0) && (tilesY > 0) && ( name != NULL )) return joinEXRs( tilesX, tilesY, name, deleteTiles, verbose);
   return 1;
}

// Return stride for the specified pixel type or 0 in case of an error
int typeSize( Imf::PixelType type)
{
   switch( type)
   {
      case Imf::UINT:  return sizeof(unsigned int);
      case Imf::HALF:  return sizeof(half);
      case Imf::FLOAT: return sizeof(float);
      case Imf::NUM_PIXELTYPES: break;
   }
   printf("This PixelType is not supported. A new one has been added?\n");
   return 0;
}

// Channel info
struct ChannelInfo
{
   Imf::Array2D<char> array2d;
   int stride;

   ChannelInfo( int inStride, int inDim1, int inDim2)
   {
      stride = inStride;
      array2d.resizeErase( inDim1 * stride, inDim2);
   }
};

// All the work is done here
int joinEXRs( int tilesX, int tilesY, const char* baseName, bool deleteTiles, bool Verbose )
{
   int exitCode = 0;

   // Expand names
   if( Verbose ) printf("Image file name = '%s', tilesX=%d, tilesY=%d\n", baseName, tilesX, tilesY);
   int numTiles = tilesX * tilesY;

   // Allocate memory:
   char ** tileNames = new char * [numTiles];
   Imf::InputFile ** iFiles = new Imf::InputFile * [numTiles];
   for( int i = 0; i < numTiles; i++)
   {
      tileNames[i] = new char[FILENAME_MAXLEN];
      iFiles[i] = 0;
   }

   // Insert tile info and check if files exist
   int nonEmptyTile = -1;
   struct stat stFileInfo;

   for( int i = 0; i < numTiles; i++)
   {
      sprintf( tileNames[i], "%s.tile_%d.exr", baseName, i);
      if( Verbose ) printf("Tile name  %d = '%s'\n", i, tileNames[i]);

      if( stat( tileNames[i], &stFileInfo ) == 0 )
      {
         // File exists - so open it and check for validness
         iFiles[i] = new Imf::InputFile( tileNames[i]);
         if( false == iFiles[i]->isComplete())
         {
            fprintf( stderr, "Error: File '%s' is incomplete or is not an OpenEXR file.\n", tileNames[i]); fflush( stderr);
            delete iFiles[i];
            iFiles[i] = 0;
            exitCode = 1;
         }
         else if( nonEmptyTile == -1 )
         {
            nonEmptyTile = i;
         }
      }
      else
      {
         fprintf( stderr, "Error: File '%s' not founded.\n", tileNames[i]); fflush( stderr);
         exitCode = 1;
      }
   }

   if( nonEmptyTile < 0) // All tiles were empty
   {
      fprintf( stderr, "Error: No tile files founded.\n"); fflush( stderr);
   }
   else
   {
   // Gather info from a non-empty tile file
   Imf::Header inHeader = iFiles[nonEmptyTile]->header();
   Imath::Box2i imageBox = inHeader.displayWindow(); // size of the resulting image
   int imageWidth = imageBox.max.x - imageBox.min.x + 1;
   int imageHeight = imageBox.max.y - imageBox.min.y + 1;

   // Iterate through all the channels and reserve mem for the whole display window
   // also add channels to the header of the output file
   Imf::Header outHeader( imageWidth, imageHeight);
   std::map< Imf::Name, ChannelInfo* > chInfos; // this will hold pixel data and stride for each channel in input files
   Imf::ChannelList channels = inHeader.channels();
   Imf::ChannelList::ConstIterator itCh;
   for( itCh = channels.begin(); itCh != channels.end(); itCh++ )
   {
      chInfos[itCh.name()] = new ChannelInfo( typeSize( itCh.channel().type), imageHeight, imageWidth );
      outHeader.channels().insert( itCh.name(), Imf::Channel( itCh.channel().type));
      if( Verbose) printf("Channel: '%s' | stride: %d\n", itCh.name(), typeSize( itCh.channel().type));
   }

   // Collect data from files
   Imath::Box2i tileBox;      // each tile's data window
   Imath::Box2i resultBox;    // resulting data window (should be sum of all tiles' data windows)
   Imf::FrameBuffer fb;
   for( int i = 0; i < numTiles; i++)
   {
      if( iFiles[i] == 0) // no file for this tile
         continue;

      tileBox = iFiles[i]->header().dataWindow();
      resultBox.extendBy( tileBox );

      if( Verbose) printf("Data win: xmin=%d xmax=%d ymin=%d ymax=%d\n", tileBox.min.x, tileBox.max.x, tileBox.min.y, tileBox.max.y);

      channels = iFiles[i]->header().channels();
      for( itCh = channels.begin(); itCh != channels.end(); itCh++ )
         fb.insert( itCh.name(),
                    Imf::Slice( itCh.channel().type,                // pixel type
                    (char*)&chInfos[itCh.name()]->array2d[0][0],    // base
                    chInfos[itCh.name()]->stride,                   // x stride
                    chInfos[itCh.name()]->stride * imageWidth,      // y stride
                    1, 1, 0.0 ) );                                  // x,y sampling, fill value

      iFiles[i]->setFrameBuffer(fb);
      iFiles[i]->readPixels( tileBox.min.y, tileBox.max.y);
   }

   // Write out everything:
   outHeader.dataWindow() = resultBox;
   Imf::OutputFile imageFile( baseName, outHeader );
   imageFile.setFrameBuffer(fb);
   imageFile.writePixels( resultBox.max.y-resultBox.min.y+1 );

   printf("Joined EXR image successfully written.\n");

   // Free files:
   for( int i = 0; i < numTiles; i++)
      if( iFiles[i] != 0 ) delete iFiles[i];
   delete [] iFiles;

   if( deleteTiles )
   {
      for( int i = 0; i < numTiles; i++)
         if( unlink( tileNames[i]) != 0)
         {
            perror("Remove");
            printf("Can't remove file '%s'\n", tileNames[i]);
         }
   }
   }

   // Free names:
   for( int i = 0; i < numTiles; i++)
      delete [] tileNames[i];
   delete [] tileNames;

   return exitCode;
}
