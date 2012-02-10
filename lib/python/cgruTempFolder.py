# -*- coding: utf-8 -*-

import os
import shutil
import stat
import sys
import time
import datetime
import cgruutils
import uuid


MAX_AGE_LOCK_FILE = 300 # Value is in minutes


class cgruTempFolder():
   # Creates a temp-folder
    def __init__( self, scene_name, service = 'generic', type = 'render', debug = False):
        self.folderPath = None
        self.debug = debug    
        self.type = type
        self.lock_uuid = str(uuid.uuid1())
        self.lock_folder = None
        self.lock_file = None
       
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
            if not creation_status:
                print("ERROR: The following Temp-Folder could not get created: %s" % self.folderPath)
        
        # If the temp-directory really exists go on
        if os.path.isdir(self.folderPath):   
            if self.debug: print("The Temp-Folder: %s" % self.folderPath)
                 
            # First create a lock-file for the folder
            self.lock_folder = os.path.join(self.folderPath, ".lock")
            self.lock_file = self.createLockFile()
            
            # And then delete all the old-temp-folders which are not in use anymore
            self.deleteOldTempFolders()

    # This method should get called after the temp-folder is not needed any more
    def closeTempFolder(self):        
        if self.lock_file != None:
            # Make sure that the lock-file gets deleted again
            self.deleteLockFile()
        else:
            print("ERROR: The Temp-Folder was already closed")
            
        # Set back the variables
        self.folderPath = None
        self.lock_folder = None
        self.lock_file = None
        

    # Returns if the temp-folder got created correctly and if it can get used
    def checkStatus(self):

        if self.lock_file == None:
            return False
        
        if self.folderPath == None:
            return False
        
        return True


    # Deleted a temp-directory. Either the one with the given path or the current one
    def deleteTempFolder(self, folder_to_delete = ""):
        if folder_to_delete == "": folder_to_delete = self.folderPath
        
        try:
            shutil.rmtree(folder_to_delete)
            if self.debug: print("The following Folder got deleted: %s" % folder_to_delete)
        except:
            # Then it seams that another task already deleted it in the meantime
            pass
        
        if self.debug: print("The following Temp-Folder got deleted: %s" % self.folderPath)


    # Checks if the temp-folder is in use (when none is given in checks the own one)
    def checkIfTempFolderInUse(self, folder_to_check = None):
        # When no folder is given check the own one
        if folder_to_check == None:
            folder_to_check = self.folderPath
        
        this_lock_directory = os.path.join(folder_to_check, ".lock")
        if os.path.isdir(this_lock_directory):
            num_lock_files = len(os.listdir(this_lock_directory))
            if num_lock_files == 0:
                return False
            else:
                return True
        else:
            # Here we return "True" just for security reasons. If that folder does not exist
            # it maybe means that there is something wrong and because this method decides
            # if a whole folder gets deleted we should keep it that way.
            return True


    def deleteOldLockFiles(self, folder_name):
        if os.path.isdir(folder_name):
            # Now get all the lock-files in the lock-file-folder to check them
            this_lock_directory = os.path.join(folder_name, ".lock")
            
            # Check first if this directory really exits
            if os.path.isdir(this_lock_directory): 

                # And if so look through the files in it
                for this_file in os.listdir(this_lock_directory):
                    this_file_path = os.path.join(this_lock_directory, this_file)
                    print(this_file_path)
                    if os.path.isfile(this_file_path):
                        # Get the age of the lock-file
                        change_time_file = int(os.path.getmtime(this_file_path))
                        
                        # Thats the maximum age of a lock-file
                        max_file_age = int(time.time()) - (MAX_AGE_LOCK_FILE * 60)
                        
                        # Check if it is older and if thats the case delete it
                        if change_time_file < max_file_age:
                            os.remove(this_file_path)
                            if self.debug: print("The following lock-file got deleted because it reached the maximum age: %s" % this_file_path)
                    else:
                        # When it is a directory and not a file delete it because there are no directories allowed in the lock-folder
                        shutil.rmtree(this_file_path)
                        if self.debug: print("The following folder got deleted because no folders are allowed in the .lock-directory: %s" % this_file_path)
                
                
                
    # Delete all old temp-folders
    # THINK ABOUT TO WRITE THIS METHOD NICER WITH SOME RECURSIVITY
    def deleteOldTempFolders(self):

        # Go through all files in the main-temp-folder
        main_temp_folder = os.path.join(self.temp_directory, self.type)       
        
        if os.path.isdir(main_temp_folder):
           
            # First go through the date directories
            files_in_main_folder = os.listdir(main_temp_folder)
            
            for this_date_file in files_in_main_folder:
                
                this_date_folder = os.path.join(main_temp_folder, this_date_file) 
                if os.path.isdir(this_date_folder):
                    
                    # Then go through the service-folders
                    files_in_date_folder = os.listdir(this_date_folder)
                    
                    #First check if the folder has any content and if not delete it
                    if len(files_in_date_folder) == 0:
                        shutil.rmtree(this_date_folder)
                        continue
                    
                    for this_service_file in files_in_date_folder:
                        this_service_folder = os.path.join(this_date_folder, this_service_file) 
                        if os.path.isdir(this_service_folder):   
                            
                            # Then go through the job-folders
                            files_in_service_folder = os.listdir(this_service_folder)
                            
                            #First check if the folder has any content and if not delete it
                            if len(files_in_service_folder) == 0:
                                shutil.rmtree(this_service_folder)
                                continue
                            
                            for this_job_file in files_in_service_folder:
                                this_job_folder = os.path.join(this_service_folder, this_job_file)
                                if os.path.isdir(this_job_folder):
                                    # First delete all the old lock-files
                                    self.deleteOldLockFiles(this_job_folder)
                                    
                                    # Then check if the folder is still in use or if it is from today. 
                                    # Only delete if nothing from both is the case
                                    if not self.checkIfTempFolderInUse(this_job_folder) and this_date_file != self.today:
                                        self.deleteTempFolder(this_job_folder)


    # Creates a lock file to know if a directory is currently in use
    def createLockFile(self):
        # Check if the lock-folder already exists and if not create it
        if not os.path.isdir(self.lock_folder):
            creation_status = cgruutils.createFolder(self.lock_folder, writeToAll = True)
        
            # If the lock-folder does still not exist inform user
            if creation_status:
                if self.debug: print("The Lock-Folder: %s" % self.lock_folder)
            else:
                print("ERROR: The following Lock-Folder could not get created: %s" % self.lock_folder)

        
        lock_file = os.path.join(self.lock_folder, self.lock_uuid)
        
        the_lock_file = open(lock_file, "w")
        the_lock_file.close()
        
        if not os.path.isfile(lock_file):
            return None
            print("ERROR: The following Lock-File could not get created: %s" % lock_file)
        
        return lock_file
        
    # Removes the current lock-file
    def deleteLockFile(self):
        os.remove(self.lock_file)