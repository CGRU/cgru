# -*- coding: utf-8 -*-

import errno
import os
import re
import shutil
import stat
import subprocess
import sys
import time
import webbrowser

import cgruconfig

ImgExtensions = ['dpx', 'exr', 'jpg', 'jpeg', 'png', 'tif', 'tiff', 'psd', 'xcf']
MovExtensions = ['mov', 'avi', 'mp4','ogg','mxf', 'mpg', 'mpeg']

def isImageExt(i_file):
    split = i_file.split('.')
    if len(split) > 1 and split[-1].lower() in ImgExtensions:
        return True
    return False

def isMovieExt(i_file):
    split = i_file.split('.')
    if len(split) > 1 and split[-1].lower() in MovExtensions:
        return True
    return False

def sepThousands(i_int):
    """Missing DocString

    :param i_int: input number
    :return: string with a space separated thousands
    """
    s = str(int(i_int)) # TODO(Why is i_int converted to int before being converted to string? This will break with . and ,)
    o = ''
    for i in range(0, len(s)):
        o += s[len(s) - i - 1]
        if (i + 1) % 3 == 0:
            o += ' '
    s = ''
    for i in range(0, len(o)):
        s += o[len(o) - i - 1]
    return s


def timecodeToFrame( i_tc, i_fps = 24):
    tc = re.findall('\d.*\d', i_tc)
    if len( tc) == 0:
        return None

    tc = re.findall('\d+', tc[0])
    if len( tc) == 0:
        return None

    frame = 0
    counts = [1,i_fps,i_fps*60,i_fps*3600]
    for i in range(0,len(tc)):
        if i >= len(counts): break
        frame += int(tc[-i-1])*counts[i]

    return frame


def timecodesToFrameRange( i_tc, i_fps = 24):
    tc = i_tc.split('-')
    if len(tc) != 2:
        return None

    return timecodeToFrame( tc[0]), timecodeToFrame( tc[1])


def timecodeFromFrame( i_frame, i_fps = 24):
    h = i_frame / (i_fps * 3600)
    f = i_frame - (i_fps * 3600) * h
    m = f / (i_fps * 60)
    f = f - (i_fps * 60 * m)
    s = f / (i_fps)
    f = f - (i_fps * s)
    return '%02d:%02d:%02d:%02d' % (h,m,s,f)


def timecodesFromFrameRange( i_ffirst, i_flast, i_fps = 24):
    tc_first = timecodeFromFrame( i_ffirst, i_fps)
    tc_last  = timecodeFromFrame( i_flast,  i_fps)
    return '%s - %s' % (tc_first,tc_last)


def copy_file(source_file, destination_file, delete_original=False, debug=False):
    """Missing DocString

    :param source_file:
    :param destination_file:
    :param delete_original:
    :param debug:
    :return:
    """
    if debug:
        print("Check-Source-File:       %s" % source_file)
        print("Check-Destination File:  %s" % destination_file)

    # Check first if file already exits
    if os.path.isfile(destination_file):
        # When it exists check if the source-filer is newer
        # We have to cast them to int because it seams windows is else more
        # exact then linux and so it makes problems
        change_time_source = int(os.path.getmtime(source_file))
        change_time_destination = int(os.path.getmtime(destination_file))

        # Only copy when the source file is newer then the destination one
        if change_time_source <= change_time_destination:
            if debug:
                print("File is not newer so continue!")
            return False

    if debug:
        print("Copy-File: %s" % source_file)

    try:
        # Now copy the file
        shutil.copy2(source_file, destination_file)

        change_time_source = int(os.path.getmtime(source_file))
        change_time_destination = int(os.path.getmtime(destination_file))
        if debug:
            print("change_time_source      : %s" % change_time_source)
            print("change_time_destination : %s" % change_time_destination)

        if change_time_source != change_time_destination:
            print('ERROR: The File "%s" did not get copied to "%s"' %
                  (source_file, destination_file))
            return False

        # When the original-file should get removed after the copy
        if delete_original:
            try:
                os.remove(source_file)
            except Exception as err:
                if err.errno is errno.ENOENT:
                    if debug:
                        print('Debug: %s was already removed.' % source_file)
                    return True
                elif err.errno is errno.EPERM or err.errno is errno.EACCES:
                    print('Warning! Could not remove %s, permission denied' % source_file)
                    return False
                else:
                    print('Warning! Unexpected error while trying to remove %s' % source_file)
                    print('Error: %s' % err)
                return False

    except Exception as err:
        if err.errno is errno.EPERM or err.errno is errno.EACCES:
            print('Warning! Could not copy %s to %s, permission denied.' % (source_file, destination_file))
        elif err.errno is errno.ENOSPC:
            print('Warning! Could not copy %s to %s, no space left on device.' % (source_file, destination_file))
        elif err.errno is errno.ENOENT:
            print('Warning! Could not copy %s to %s, folder not found.' % (source_file, destination_file))
            print('Trying to create folder %s. ' % os.path.dirname(destination_file))
            try:
                os.makedirs(os.path.dirname(destination_file))
                print('Directory created, trying to copy file again.')
                copy_file(source_file, destination_file, delete_original, debug)
                return True
            except Exception as err:
                if err.errno is errno.EPERM or err.errno is errno.EACCES:
                    print('Warning! Could not create directory %s, permission denied' % os.path.dirname(destination_file))
                elif err.errno is errno.ENOSPC:
                    print('Warning! Could not create direcotry %s, no space left on device.' % os.path.dirname(destination_file))
                elif err.errno is errno.EROFS:
                    print('Warning! Could not create direcotry %s, read-only file system.' % os.path.dirname(destination_file))
                else:
                    print('Warning! Unexpected error while trying to remove %s' % source_file)
                    print('Error: %s' % err)
        return False

    return True


def copy_directory(source_folders, destination_folder, reg_match=None, delete_copied_files=False, debug=False):
    """Missing DocString

    :param source_folders:
    :param destination_folder:
    :param reg_match:
    :param delete_copied_files:
    :param debug:
    :return:
    """
    copied_files = 0

    # That it can it can also work when "source_folders" is not a list
    if type(source_folders) is not list:
        source_folders = [source_folders]

    # Make sure the destination directory really exits
    if not os.path.isdir(destination_folder):
        creation_status = createFolder(destination_folder, writeToAll=True)

        if creation_status:
            if debug:
                print("The Destination-Folder did not exist and did get "
                      "created: %s" % destination_folder)
        else:
            print("Warning: The following Destination-Folder could not get "
                  "created so the files could not get copied: %s" %
                  destination_folder)
            return copied_files

    # Go through all source-folders
    for source_folder in source_folders:
        # Exit when the source_folder does not exist or is no folder
        if not os.path.isdir(source_folder):
            print("Warning: Source-Folder could not get copied because it is "
                  "either no folder or it does not exist: %s" % source_folder)
            return copied_files

        # Get the directory content, files & folders
        listing = os.listdir(source_folder)

        # Go through all files & directories in source-directory
        for file_name in listing:
            # First check if it is file or directory
            if os.path.isdir(os.path.join(source_folder, file_name)):
                # When it is directory call itself with new parameters
                copied_files += copy_directory(
                    os.path.join(source_folder, file_name),
                    os.path.join(destination_folder, file_name),
                    delete_copied_files=delete_copied_files, debug=debug)
            else:
                # When it is a file
                if reg_match:
                    # When reg_match is given if the file matches and if not
                    # skip it
                    if not re.match(reg_match, file_name):
                        continue

                # Build the full-file-paths
                source_file = os.path.join(source_folder, file_name)
                destination_file = os.path.join(destination_folder, file_name)

                # Call copy-function which only copies when file is newer
                if copy_file(source_file, destination_file,
                             delete_original=delete_copied_files, debug=debug):
                    copied_files += 1
                else:
                    continue

    return copied_files


def createFolder(path, writeToAll=True):
    """Missing DocString

    :param path:
    :param writeToAll:
    :return:
    """
    status = True
    if not os.path.isdir(path):

        try:
            os.makedirs(path)
        except Exception as err:
            if err.errno is errno.EEXIST:
                status = True
                pass # Folder already exists so ignore the error
            elif err.errno is errno.EPERM or err.errno is errno.EACCES:
                print('Warning! Could not create direcotry %s, permission denied.' % path)
                status = False
            elif err.errno is errno.ENOSPC:
                print('Warning! Could not create directory %s, no space left on device.' % path)
                status = False
            elif err.errno is errno.EROFS:
                print('Warning! Could not create directory %s, read-only file system.' % path)
                status = False
            else:
                print('Warning! Unexpected error while trying to create %s.' % path)
                print('Error: %s' % err)
                status = False

    if status and writeToAll:
        try:
            os.chmod(path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
        except Exception as err:
            if err.errno is errno.EPERM or err.errno is errno.EACCES:
                print('Warning! Could not change permissions for %s, permission denied.')
            else:
                print('Warning! Unexpected error while trying to change permissions for %s.' % path)
                print('Error: %s' % err)

    return status


def getIconFileName(iconname):
    """Missing DocString

    :param iconname:
    :return:
    """
    icon_path = os.path.join(
        os.path.join(
            cgruconfig.VARS['CGRU_LOCATION'],
            'icons'
        )
    )
    icon_paths = cgruconfig.VARS['icons_path']
    if icon_paths is None:
        icon_paths = icon_path

    if icon_paths.find(';') != -1:
        icon_paths = icon_paths.split(';')
    elif sys.platform.find('win') == -1:
        icon_paths = icon_paths.split(':')
    else:
        icon_paths = [icon_paths]

    if not icon_path in icon_paths:
        icon_paths.append(icon_path)

    for icon_path in icon_paths:
        icon_path = os.path.join(icon_path, iconname)
        if os.path.isfile(icon_path):
            return icon_path

        icon_path += '.png'

        if os.path.isfile(icon_path):
            return icon_path

    return None


def copyJobFile(fileName, jobName='', fileExtension=''):
    """Missing DocString

    :param fileName:
    :param jobName:
    :param fileExtension:
    :return:
    """
    copyFile = ''
    if not os.path.isfile(fileName):
        print('Error: no file "%s"' % fileName)
    else:
        copyFile = fileName
        if jobName != '':
            copyFile += '.' + jobName
        ftime = time.time()
        copyFile += '.' + time.strftime('%m%d-%H%M%S-') + str(
            ftime - int(ftime))[2:5]
        if fileExtension != '':
            copyFile += '.' + fileExtension
        print('Copying to ' + copyFile)
        try:
            shutil.copyfile(fileName, copyFile)
        except Exception as err:
            if err.errno is errno.EPERM or err.errno is errno.EACCES:
                print('Warning! Could not copy %s to %s, permission denied.' % (fileName, copyFile))
            elif err.errno is errno.ENOSPC:
                print('Warning! Could not copy %s to %s, no space left on device.' % (fileName, copyFile))
            elif err.errno is errno.ENOENT:
                print('Warning! Could not copy %s to %s, folder not found.' % (fileName, copyFile))
                print('Trying to create folder %s. ' % os.path.dirname(copyFile))
                try:
                    os.makedirs(os.path.dirname(copyFile))
                    print('Directory created, trying to copy file again.')
                    copyJobFile(fileName, jobName, fileExtension)
                    return copyFile
                except Exception as err:
                    if err.errno is errno.EPERM or err.errno is errno.EACCES:
                        print('Warning! Could not create directory %s, permission denied' % os.path.dirname(copyFile))
                    elif err.errno is errno.ENOSPC:
                        print('Warning! Could not create direcotry %s, no space left on device.' % os.path.dirname(copyFile))
                    elif err.errno is errno.EROFS:
                        print('Warning! Could not create direcotry %s, read-only file system.' % os.path.dirname(copyFile))
                    else:
                        print('Warning! Unexpected error while trying to remove %s' % fileName)
                        print('Error: %s' % err)
                    copyFile = ''
            copyFile = ''
    return copyFile


def toStr(data):
    """Missing DocString

    :param data:
    :return:
    """
    if isinstance(data, str):
        return data

    if sys.version_info[0] < 3:
        if isinstance(data, unicode):
            return data.encode('utf-8')
        return str(data)

    return str(data, 'utf-8', 'replace')


def webbrowse(i_url):
    """Missing DocString

    :param i_url:
    :return:
    """
    if i_url[:4] != 'http':
        i_url = 'http://' + i_url

    if 'webbrowser' in cgruconfig.VARS:
        cmd = '"%s"' % cgruconfig.VARS['webbrowser']
        if cmd.find('%s') != -1:
            cmd = cmd % i_url
        else:
            cmd = '%s "%s"' % (cmd, i_url)

        if 'windows' in cgruconfig.VARS['platform']:
            cmd = 'start "Web GUI" ' + cmd
        else:
            cmd += ' &'

        subprocess.Popen( cmd, shell=True)

        return

    webbrowser.open(i_url)
