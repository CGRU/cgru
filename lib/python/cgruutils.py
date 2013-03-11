# -*- coding: utf-8 -*-

import datetime
import os
import shutil
import stat
import sys
import time
import re
import webbrowser

def copy_file(source_file, destination_file, delete_original = False, debug=False):
    
    if debug:
        print("Check-Source-File:       %s" % source_file)
        print("Check-Destination File:  %s" % destination_file)
    
    # Check first if file already exits
    if os.path.isfile(destination_file):
        # When it exists check if the source-filer is newer
        # We have to cast them to int because it seams windows is else more exact then linux and so it makes problems
        change_time_source = int(os.path.getmtime(source_file))
        change_time_destination = int(os.path.getmtime(destination_file))
        
        # Only copy when the source file is newer then the destination one
        if change_time_source <= change_time_destination:
            if debug: print("File is not newer so continue!")
            return False
    
    if debug: print("Copy-File: %s" % source_file)      
    
    try:
        # Now copy the file               
        shutil.copy2(source_file, destination_file)

        change_time_source = int(os.path.getmtime(source_file))
        change_time_destination = int(os.path.getmtime(destination_file))
        if debug:
            print("change_time_source      : %s" % change_time_source)
            print("change_time_destination : %s" % change_time_destination)
        
        if change_time_source != change_time_destination:
             print('ERROR: The File "%s" did not get copied to "%s"' % (source_file, destination_file))
             return False
            
        # When the original-file should get removed after the copy
        if delete_original:
            try:
                os.remove(source_file)
            except:
                return False
    except:
        return False
    
    return True


def copy_directory(source_folders, destination_folder, reg_match = None, delete_copied_files = False, debug=False):
    copied_files = 0

    # That it can it can also work when "source_folders" is not a list
    if type(source_folders) is not list:
        source_folders = [source_folders]

    # Make sure the destination directory really exits
    if not os.path.isdir(destination_folder):
        
        creation_status = createFolder( destination_folder, writeToAll = True)
        
        if creation_status:
            if debug: print("The Destination-Folder did not exist and did get created: %s" % destination_folder)
        else:
            print("Warning: The following Destination-Folder could not get created so the files could not get copied: %s" % destination_folder)
            return copied_files            
        
        
    # Go through all source-folders
    for source_folder in source_folders:
        # Exit when the source_folder does not exist or is no folder
        if not os.path.isdir(source_folder):
            print("Warning: Source-Folder could not get copied because it is either no folder or it does not exist: %s" % source_folder)
            return copied_files
        
        # Get the directory content, files & folders
        listing = os.listdir(source_folder)

        # Go through all files & directories in source-directory        
        for file_name in listing:
             # First check if it is file or directory
            if os.path.isdir(os.path.join(source_folder, file_name)):
                # When it is directory call itself with new parameters
                copied_files += copy_directory(os.path.join(source_folder, file_name), os.path.join(destination_folder, file_name), delete_copied_files=delete_copied_files, debug=debug)
            else:
                # When it is a file
                if reg_match:
                    # When reg_match is given if the file matches and if not skip it
                    if not re.match(reg_match, file_name):
                        continue
                
                # Build the full-file-paths
                source_file = os.path.join(source_folder, file_name)
                destination_file = os.path.join(destination_folder, file_name)

                # Call copy-function which only copies when file is newer
                if copy_file(source_file, destination_file, delete_original=delete_copied_files, debug=debug):
                    copied_files += 1
                else:
                    continue

    
    return copied_files



def createFolder( path, writeToAll = True):
   status = True
   if not os.path.isdir( path):
       
      try:
         os.makedirs( path )
      except:       
         # If it did not work we check if it maybe already did exist because another task did create it in the meantime
         if not os.path.isdir(path):
            # Ok now we are sure that there is a "real" problem because the order does still not exist
            print(str(sys.exc_info()[1]))
            status = False
   if status and writeToAll:
      try:
         os.chmod( path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
      except:
         print(str(sys.exc_info()[1]))
   return status

import cgruconfig

def getIconFileName( iconname):
   icon_path = os.path.join( os.path.join( cgruconfig.VARS['CGRU_LOCATION'],'icons'))
   icon_paths = cgruconfig.VARS['icons_path']
   if icon_paths is None: icon_paths = icon_path
   if icon_paths.find(';') != -1: icon_paths = icon_paths.split(';')
   elif sys.platform.find('win') == -1: icon_paths = icon_paths.split(':')
   else: icon_paths = [icon_paths]
   if not icon_path in icon_paths: icon_paths.append( icon_path)
   for icon_path in icon_paths:
      icon_path = os.path.join( icon_path, iconname)
      if os.path.isfile( icon_path): return icon_path
      icon_path += '.png'
      if os.path.isfile( icon_path): return icon_path
   return None

def copyJobFile( fileName, jobName = '', fileExtension = ''):
   copyFile = ''
   if not os.path.isfile( fileName):
      print('Error: no file "%s"' % fileName)
   else:
      copyFile = fileName
      if jobName != '': copyFile += '.' + jobName
      ftime = time.time()
      copyFile += '.' + time.strftime('%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5]
      if fileExtension != '': copyFile += '.' + fileExtension
      print('Copying to ' + copyFile)
      try:
         shutil.copyfile( fileName, copyFile)
      except:
         print(str(sys.exc_info()[1]))
         copyFile = ''
   return copyFile

def toStr( data):
	if isinstance( data, str):
		return data

	if sys.version_info[0] < 3:
		if isinstance( data, unicode):
			return data.encode('utf-8')
		return str( data)

	return str( data, 'utf-8')

def webbrowse( i_url):
	if i_url[:7] != 'http://':
		i_url = 'http://'+i_url

	if 'webbrowser' in cgruconfig.VARS:
		cmd = '"%s"' % cgruconfig.VARS['webbrowser']
		if cmd.find('%s') != -1:
			cmd = cmd % i_url
		else:
			cmd = '%s "%s"' % ( cmd, i_url)
		if 'windows' in cgruconfig.VARS['platform']:
			cmd = 'start "Web GUI" '+cmd
		else:
			cmd += ' &'
		os.system( cmd)
		return

	webbrowser.open( i_url)

