import os
import subprocess
import sys
import re
import cgruutils
import cgruTempFolder
#-------------------------------------------------------------------------------------
#                  FIRST THE FUNCTIONS
#-------------------------------------------------------------------------------------



def error_exit( msg ):
   print("ERROR: %s" % msg)
   sys.stdout.flush()
   exit(1)


#-------------------------------------------------------------------------------------
#                  NOW START WITH THE ACTUAL SCRIPT
#-------------------------------------------------------------------------------------

# Get all the parameters given by the user
parameteres = sys.argv


# Get hust the parameters needed, all the other ones get passed directly to c4d and let it take care about it
try:
    parameteres.index("-debug")
    debug = True
except:
    debug = False

scene_file_path = parameteres[parameteres.index("-render")+1]

# Check the only argument that we care about the scenefile
if scene_file_path == '': error_exit('Scene to execute is not specified.')


# Get Afanasy root directory:
afroot = os.getenv('AF_ROOT')
if afroot is None: error_exit('AF_ROOT is not defined.')


## Get the scene-path and the the scene-file-name
(scene_folder, scene_file) = os.path.split(scene_file_path)


# Create and check the c4d-temp directory:
my_temp_directory = cgruTempFolder.cgruTempFolder(scene_file, service = 'c4d', type = 'render', debug = debug)
temp_directory = my_temp_directory.folderPath


# When there was a problem with creating the temp-directory then error-out
if not temp_directory:
    error_exit('Error creating temp directory.')


# Create the temp-scene-file-path that we know where to copy it to
temp_scene_file_path = os.path.join( temp_directory, scene_file)


# Copy the scene-file to the temp-directory (copies only when newer then a maybe already existing one)
cgruutils.copy_file(scene_file_path, temp_scene_file_path, debug=debug)


# Copy now the scene-folders (copies only when newer then a maybe already existing one)
folders_to_copy = ['tex']
for this_folder_name in folders_to_copy:
    source_folder = os.path.join(scene_folder, this_folder_name)
    temp_folder = os.path.join(temp_directory, this_folder_name)
    cgruutils.copy_directory(source_folder, temp_folder, debug=debug)


# Now create the folder in which always the images get rendered to
temp_output_folder = os.path.join(temp_directory, "Output")
if not os.path.isdir(temp_output_folder):
    creation_status = cgruutils.createFolder( temp_output_folder, writeToAll = True)
    
    if not creation_status:
        error_exit( "The Temp-Output-Folder could not get created. So images can not get rendered and script stopped" )

# Create the parameteres to execute and set that it should start with the c4d command and should use the new scene-file
parameteres = sys.argv
parameteres[0] = "c4d"
parameteres[parameteres.index("-render")+1] = temp_scene_file_path


output_overwrite = False
# If image-output-overwrite is set then redirect it to local Output-Folder
try:
    output_overwrite = parameteres[parameteres.index("-oimage")+1]
    (oimage_overwrite_folder, oimage_overwrite_filename) = os.path.split(output_overwrite)
    parameteres[parameteres.index("-oimage")+1] = os.path.join(temp_output_folder, oimage_overwrite_filename)
except:
    oimage_overwrite_folder = False


# If image-omultipass-overwrite is set then redirect it to local Output-Folder
try:
    output_overwrite = parameteres[parameteres.index("-omultipass")+1]
    (omultipass_overwrite_folder, omultipass_overwrite_filename) = os.path.split(output_overwrite)
    parameteres[parameteres.index("-omultipass")+1] = os.path.join(temp_output_folder, omultipass_overwrite_filename)
except:
    omultipass_overwrite_folder = False



# We have to be in the correct working-directory
os.chdir(temp_directory)

print("Running Command: %s" % (' '.join(parameteres)))
sys.stdout.flush()

# Now execute the command
process = subprocess.Popen( parameteres, shell=True, stdout=subprocess.PIPE)


re_frame   = re.compile(r'Rendering frame \d+')
re_number  = re.compile(r'\d+')
rendered_frames = []


#process.wait()

# Output what is happening
while True:
    data = process.stdout.readline().decode("utf-8")
    print(str(data))

    # Now update the some Informations like current frame & percent
    match = re_frame.search( data )  
    
    if match:
        # Get the current frame-number
        frame = re_number.search( match.group(0))
        this_frame = int(frame.group(0))
        
        # Now add it to the array that we know that it got rendered
        rendered_frames.append(str(this_frame))
    
    sys.stdout.flush()
    if not data: break
 
#process.wait()


copied_files = 0
# Now copy the rendered files to the server

if output_overwrite:
    # When the output got overwritten copy it where it was defined by the used by the overwrite
    if oimage_overwrite_folder:
        output_folder = oimage_overwrite_folder
    if omultipass_overwrite_folder:
        output_folder = omultipass_overwrite_folder
else:
    # When the output was not overwritten copy it in the Output-Folder of the original scene-file
    output_folder = os.path.join(scene_folder, 'Output')

# To find the correct files to copy
reg_match = '.*([a-zA-Z]|\.)+(0?|0+)(%s)\.[a-zA-Z0-9]{1,5}$' % ("|".join(rendered_frames))

# Now actually copy and then delete the renderings
copied_files = cgruutils.copy_directory(temp_output_folder, output_folder, reg_match=reg_match, delete_copied_files=True, debug=debug)

print("Output-Files copied: %d" % copied_files)


# Check if really files got copied
if copied_files == 0:
    error_exit('No Rendered-Images got copied')   
elif copied_files < len(rendered_frames):
    error_exit('Not all rendered Images got copied') 


# Now lets tell the temp-folder that we do not need it any more
my_temp_directory.closeTempFolder()

# If we did get till here it means everything went well    
print("FINISHED WITH SCRIPT")