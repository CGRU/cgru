#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import stat
import sys
import time

import mediainfo

import cgruutils

from optparse import OptionParser

Parser = OptionParser(
    usage="%prog [options]\ntype \"%prog -h\" for help",
    version="%prog 1.0"
)

Parser.add_option('-o', '--output',   dest='output',   type = 'string',     default='.rules/walk.json', help='File to save results.')
Parser.add_option('-n', '--noupdate', dest='noupdate', action='store_true', default=False,              help='Skip update upfolders.')
Parser.add_option('-m', '--mediainfo',dest='mediainfo',action='store_true', default=False,              help='Get media information.')
Parser.add_option('-t', '--thumb',    dest='thumb',    type = 'int',        default=None,               help='Make thumbnail frequency.')
Parser.add_option('-r', '--report',   dest='report',   type = 'int',        default=None,               help='Print report frequency.')
Parser.add_option('-V', '--verbose',  dest='verbose',  type = 'int',        default=0,                  help='Verbose mode.')
Parser.add_option('-D', '--debug',    dest='debug',    action='store_true', default=False,              help='Debug mode.')

Options, Args = Parser.parse_args()

Progress = 0
PrevFiles = None
CurFiles = 0
StartPath = '.'
ThumbFolderCount = 0
ReportFolderCount = 0
TotalSpace = 0
os.umask(0000)

print('{"walk":{')

def outInfo( i_key, i_msg):
    print(' "%s"\t:"%s",' % (i_key, i_msg))
    sys.stdout.flush()

def outStatus( i_status):
    print(' "status"\t:"%s"' % i_status)
    print('}}')

def getSizeSpace( i_st):
    """ Return files size and space (disk usage)
        :param i_st: stat structure (os.stat())
        :return: tuple (size,space)
        http://man7.org/linux/man-pages/man2/stat.2.html
    """
    size = i_st.st_size
    space = i_st.st_blocks * 512
    return size, space

if len(Args):
    StartPath = Args[0]

outInfo('path',StartPath)

if not os.path.isdir(StartPath):
    outInfo('error','Start path does not exists.')
    outStatus('error')
    sys.exit(1)

def jsonLoad(i_filename):
    if not os.path.isfile(i_filename):
        return None

    try:
        file = open(i_filename, 'r')
    except:
        outInfo('error_file_open',str(sys.exc_info()[1]))
        return None

    obj = None
    try:
        obj = json.load(file)
    except:
        outInfo('error_file_json',str(sys.exc_info()[1]))
        obj = None

    file.close()

    return obj

def checkDict(io_dict, i_reset_counts = False):
    if not 'folders' in io_dict:
        io_dict['folders'] = dict()
    if not 'files' in io_dict:
        io_dict['files'] = dict()
    num_keys = ['num_files', 'num_folders','num_images', 'size', 'num_files_total',
                'num_folders_total', 'size_total','space']
    for key in num_keys:
        if i_reset_counts:
            io_dict[key] = 0
        else:
            if not key in io_dict:
                io_dict[key] = 0


def walkdir(i_path, i_subwalk, i_curdepth=0):
    global Progress
    global PrevFiles
    global CurFiles
    global ThumbFolderCount
    global ReportFolderCount
    global TotalSpace

    # Output current path:
    if Options.verbose > i_curdepth and i_subwalk:
        outInfo('cur_path',i_path)

    # Output report:
    if Options.report is not None:
        if i_path.find( os.path.dirname( Options.output)) == -1:
            if ReportFolderCount % Options.report == 0:
                print('REPORT: %s - %.1f GB' % ( i_path, TotalSpace / 1024 / 1024 / 1024 ))
                sys.stdout.flush()
            ReportFolderCount += 1

    out = jsonLoad( os.path.join( i_path, Options.output))
    if out is None:
        out = dict()
    checkDict(out, True)

    try:
        entries = os.listdir(i_path)
    except:
        outInfo('error_listdir',str(sys.exc_info()[1]))
        return None

    for entry in entries:

        path = os.path.join(i_path, entry)

        st = None
        try:
            st = os.lstat( path)
        except:
            outInfo('error_listdir',str(sys.exc_info()[1]))
            continue

        # We are not walking in links:
        if stat.S_ISLNK( st.st_mode):
            continue

        if stat.S_ISDIR( st.st_mode):
            out['num_folders'] += 1
            out['num_folders_total'] += 1
            size, space = getSizeSpace( st)
            out['space'] += space
            TotalSpace += space

            fout = None
            if i_subwalk:
                # Recursively walk in a subfolder:
                fout = walkdir(path, True, i_curdepth + 1)
            else:
                # Load previous walk data:
                fout = jsonLoad(os.path.join(path, Options.output))

            if fout is not None:
                checkDict(fout)

                # We do not need info for each subfolder in a child folder:
                del fout['files']
                del fout['folders']

                # Create an empty folder entry if not preset:
                if not entry in out['folders']:
                    out['folders'][entry] = dict()

                # Merge keys from subfolder:
                for key in fout.keys():
                    out['folders'][entry][key] = fout[key]

                out['num_folders_total'] += fout['num_folders_total']
                out['num_files_total'] += fout['num_files_total']
                out['size_total'] += fout['size_total']
                out['space'] += fout['space']

        if stat.S_ISREG( st.st_mode):
            CurFiles += 1
            size, space = getSizeSpace( st)
            if entry[0] != '.':
                out['num_files'] += 1
                if cgruutils.isImageExt( path):
                    if Options.thumb is not None:
                        if out['num_images'] == 0:
                            if ThumbFolderCount % Options.thumb == 0 and size < 10000000:
                                print('@IMAGE!@'+path)
                                sys.stdout.flush()
                            ThumbFolderCount += 1
                    out['num_images'] += 1
                elif cgruutils.isMovieExt( path) and Options.mediainfo:
                    obj = mediainfo.processMovie( path)
                    if obj and 'mediainfo' in obj:
                        out['files'][entry] = obj['mediainfo']
            out['num_files_total'] += 1
            out['size_total'] += size
            out['size'] += size
            out['space'] += space
            TotalSpace += space

    # Just output progress:
    if PrevFiles:
        cur_progress = int(100.0 * CurFiles / PrevFiles)
        if cur_progress != Progress:
            Progress = cur_progress
            outInfo('progress','PROGRESS: %d%%' % Progress)

    # Skip soting data in .rules folders, or we will create '.rules/.rules' folders
    if os.path.basename(i_path) == os.path.dirname(Options.output):
        return out
            
    # Store current walk data:
    filename = os.path.join(i_path, Options.output)

    if not os.path.isdir(os.path.dirname(filename)):
        try:
            os.makedirs(os.path.dirname(filename))
        except:
            outInfo('error_make_dir',str(sys.exc_info()[1]))

    if os.path.isdir(os.path.dirname(filename)):
        try:
            with open(filename, 'w') as f:
                json.dump(out, f, indent=1)
        except:
            outInfo('error_file_write',str(sys.exc_info()[1]))

    return out


# #############################################################################
time_start = time.time()
outInfo('time_start', time.ctime(time_start))

# Get old files count if any:
prev = jsonLoad(os.path.join(StartPath, Options.output))
if prev is not None:
    if 'num_files_total' in prev:
        checkDict(prev)
        PrevFiles = prev['num_files_total']

if PrevFiles:
    outInfo('previous','%s files, %s folders, %s bytes' % (
        cgruutils.sepThousands(prev['num_files_total']), cgruutils.sepThousands(prev['num_folders_total']),
        cgruutils.sepThousands(prev['size_total'])))

# Walk in subfolders:
walk = walkdir(StartPath, True)

# Calculate difference with previous
d_files = None
d_folders = None
d_size = None
if PrevFiles:
    d_files = walk['num_files_total'] - prev['num_files_total']
    d_folders = walk['num_folders_total'] - prev['num_folders_total']
    d_size = walk['size_total'] - prev['size_total']


# Update parent folders:
if not Options.noupdate:
    curpath = StartPath
    PrevFiles = None
    while curpath != '/' and curpath != '':
        # Go one folder upper:
        uppath = os.path.dirname(curpath)
        if uppath == curpath or uppath == '' or uppath == '/':
            break
        curpath = uppath

        outInfo('updating', curpath)
        walkdir(curpath, False)


# Output statistics:
time_finish = time.time()
outInfo('time_finish', time.ctime(time_finish))
outInfo('processed','%s files, %s folders, %s bytes' % (
    cgruutils.sepThousands(walk['num_files_total']),
    cgruutils.sepThousands(walk['num_folders_total']),
    cgruutils.sepThousands(walk['size_total']))
)
print('"walk":%s,' % json.dumps(walk,indent=1))

if d_files is not None:
    outInfo('delta','%s files, %s folders, %s bytes' %
        (cgruutils.sepThousands(d_files), cgruutils.sepThousands(d_folders), cgruutils.sepThousands(d_size)))

sec = time_finish - time_start
hrs = int(sec / 3600)
sec -= hrs * 3600
mns = int(sec / 60)
sec -= mns * 60
msc = int(1000.0 * sec - int(sec))
sec = int(sec)
outInfo('time_run','%02d:%02d:%02d.%03d' % (hrs, mns, sec, msc))

outStatus('success')

if Options.report:
    print('REPORT: %.1f GB' % (walk['space'] / 1024.0 / 1024.0 / 1024.0))
