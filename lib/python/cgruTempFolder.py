# -*- coding: utf-8 -*-

import os
import shutil
import stat
import sys
import time
import datetime
import cgruutils

class cgruTempFolder():
   # Creates a temp-folder
    def __init__( self, scene_name, service = 'generic', type = 'render', debug = False):
        self.folderPath = None
        self.debug = debug    
        self.type = type
       
        # Try to get the path to the main-temp-folder
        try:
            self.temp_directory = os.getenv('TMP')
        except:
            try:
                self.temp_directory = os.getenv('TEMP')
            except:
                self.temp_directory = os.path.join(os.getenv('HOME'), "tmp")

        # Get todays and yesterdays date for temp-folder-name and to know which ones to keep when the delete-old-temp-folder function runs
        today_time = datetime.datetime.today()
        yesterday_time = today_time - datetime.timedelta(1)
        
        self.today = today_time.strftime("%Y-%m-%d")
        self.yesterday = yesterday_time.strftime('%Y-%m-%d')

    
        # Build the full-temp-path 
        self.folderPath = os.path.join(self.temp_directory, self.type, self.today, service, scene_name)

        # Check if the temp-folder already exists and if not create it
        if not os.path.isdir(self.folderPath):
            creation_status = cgruutils.createFolder( self.folderPath, writeToAll = True)
        
            # If the temp-folder does still not exist inform user
            if creation_status:
                if self.debug: print("The Temp-Folder: %s" % self.folderPath)
            else:
                print("ERROR: The following Temp-Folder could not get created: %s" % self.folderPath)


    # Deleted a temp-directory. Either the one with the given path or the current one
    def deleteTempFolder(self, folder_to_delete = ""):
        if folder_to_delete == "": folder_to_delete = self.folderPath
        
        try:
            shutil.rmtree(folder_to_delete)
        except:
            # Then it seams that another task already deleted it in the meantime
            pass
        
        if self.debug: print("The following Temp-Folder got deleted: %s" % self.folderPath)


    # Delete all old temp-folders
    def deleteOldTempFolders(self):
        
        # Go through all files in the main-temp-folder
        for this_file in os.listdir(os.path.join(self.temp_directory, self.type)):

            # Only delete if fFolder and not the currenlty used one
            this_folder = os.path.join(self.temp_directory, self.type, this_file)
            if os.path.isdir(this_folder) and this_file != self.today and this_file != self.yesterday:
                # Now call the delete-method
                self.deleteTempFolder(this_folder)
