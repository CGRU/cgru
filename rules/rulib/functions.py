import cgi
import json
import getpass
import os
import shutil
import sys
import time
import traceback

import rulib

import cgruutils

import mediainfo

def randMD5():
    hashlib = __import__('hashlib', globals(), locals(), [])
    random = __import__('random', globals(), locals(), [])
    return hashlib.md5(str(random.random()).encode()).hexdigest()

def getCurSeconds():
    return round(time.time())

def getCurMSeconds():
    return round(time.time() * 1000)

def getCurUser():
    return getpass.getuser()

def outError(i_err, o_out = None):
    if o_out is not None:
        o_out['error'] = i_err
    else:
        print(i_err)

def getRuFilePath(i_file, i_path = None):
    path = i_path
    if path is None:
        path = os.getcwd()
    path = os.path.join(path, rulib.RUFOLDER, i_file)
    path = getAbsPath(path)
    return path


def getRootPath(i_path = None):
    path = i_path
    if path is None:
        path = os.getcwd()
    else:
        path = os.path.abspath(path)
    if path.find(rulib.ROOT) == 0:
        path = path[len(rulib.ROOT):]
    return path


def getAbsPath(i_path):
    if len(i_path) == 0:
        return rulib.ROOT
    if i_path[0] == '/':
        return rulib.ROOT + i_path
    return rulib.ROOT + '/' + i_path

def isAuxFolder(i_path, i_status = None):
    if i_status is not None:
        if 'flags' in i_status and 'aux' in i_status['flags']:
            return True

        if 'progress' in i_status and type(i_status['progress']) is int and i_status['progress'] < 0:
            return True

    name = os.path.basename(i_path).lower()
    for aux in rulib.RULES_TOP['aux_folders']:
        if name.find(aux) == 0:
            return True

    return False

def getRuFiles(i_path = None, i_ruFolder = None):
    if i_path is None: i_path = os.getcwd()
    if i_ruFolder is None: i_ruFolder = rulib.RUFOLDER

    ruFolder = os.path.join(i_path, i_ruFolder)

    if not os.path.isdir(ruFolder):
        return []
    ruFiles = []
    for afile in os.listdir(ruFolder):
        if afile.find('rules') != 0:
            continue
        if afile.find('.json') == -1:
            continue
        ruFiles.append(os.path.join(ruFolder, afile))
    ruFiles.sort()

    return ruFiles

def getRulesUno(i_path = None, i_ruFolder =  None):
    rules = dict()
    for afile in getRuFiles(i_path, i_ruFolder):
        out = dict()
        obj = readObj(afile, out)
        if obj is None:
            errobj = dict()
            errobj['error'] = 'Invalid rules file: "%s"' % afile
            if 'error' in out:
                errobj['error'] = out['error']
            if 'info' in out:
                errobj['info'] = out['info']
            rules['ruerror'] = errobj
            break
        rulib.editobj.mergeObjs(rules, obj)
    return rules

def fileRead(i_file, i_lock = True, i_verbose = False):
    try:
        f = open(i_file, mode='r', encoding='utf-8')
    except:
        return None

    data = f.read(rulib.FILE_MAX_LENGTH)
    f.close()

    if i_verbose:
        print('fileRead: Read %d bytes from %s.' % (len(data), i_file))

    return data

def fileWrite(i_file, i_data, o_out=None):
    tmp_name = ('%s-%s') % (i_file, os.getpid())
    try:
        if isinstance(i_data, str):
            f = open(tmp_name, mode='w', encoding='utf-8')
        else:
            f = open(tmp_name, mode='wb')
    except PermissionError:
        err = 'Permissions denied to write file: ' + tmp_name
        if o_out is not None: o_out['error'] = err
        else: print(err)
        return False
    except:
        err = 'Unable open for writing: ' + tmp_name
        err += '\n%s' % traceback.format_exc()
        if o_out is not None: o_out['error'] = err
        else: print(err)
        return False

    #if ($i_lock) _flock_($fHandle, LOCK_EX)
    f.write(i_data)
    #if ($i_lock) _flock_($fHandle, LOCK_UN)
    f.close()

    os.rename(tmp_name, i_file)

    return True


def readObj(i_file, o_out = None, i_lock = True):
    if not os.path.isfile(i_file):
        error = 'No such file %s' % i_file
        if o_out is not None:
            o_out['error'] = error
        return

    data = fileRead(i_file, i_lock)
    obj = None

    if data is None:
        error = 'Unable to read file %s' % i_file
        if o_out is not None:
            o_out['error'] = error
        return

    try:
        obj = json.loads(data)
    except:
        obj = None
        if o_out is not None:
            o_out['error'] = 'Can`t read json object from "%s".' % i_file
            o_out['info'] = '%s' % traceback.format_exc()

    return obj


def writeObj(i_file, i_obj, o_out=None):
    if fileWrite(i_file, json.dumps(i_obj, indent='\t', sort_keys=False), o_out):
        return True

    return False


def readConfig(i_file, o_out):

    if not os.path.isfile(i_file):
        o_out['error'] = 'No such config file: %s in %s' % (i_file, os.getcwd())
        return

    obj = readObj(i_file, o_out)
    if 'error' in o_out:
        return

    if obj:
        o_out[i_file] = obj
        if 'include' in obj['cgru_config']:
            for afile in obj['cgru_config']['include']:
                readConfig(afile, o_out)


def readUser(i_uid, i_full):
    ufile_main = '%s/users/%s/%s.json' % (rulib.CGRU_LOCATION, i_uid, i_uid)
    ufile_news = '%s/users/%s/%s-news.json' % (rulib.CGRU_LOCATION, i_uid, i_uid)
    ufile_bookmarks = '%s/users/%s/%s-bookmarks.json' % (rulib.CGRU_LOCATION, i_uid, i_uid)

    if not os.path.isfile(ufile_main):
        return None

    user = readObj(ufile_main)
    if user is None:
        return None

    # Not full request does not contain news and bookmarks
    if not i_full:
        return user

    # Auxiliary user state means no news and bookmarks
    if 'states' in user and 'aux' in user['states']:
        return user

    if os.path.isfile(ufile_news):
        news = readObj(ufile_news)
        if news:
            if 'news' in news and len(news['news']):
                user['news'] = news['news']
            else:
                os.unlink(ufile_news)
        else:
            os.unlink(ufile_news)

    if os.path.isfile(ufile_bookmarks):
        bookmarks = readObj(ufile_bookmarks)
        if bookmarks:
            if 'bookmarks' in bookmarks and len(bookmarks['bookmarks']):
                user['bookmarks'] = bookmarks['bookmarks']
            else:
                os.unlink(ufile_bookmarks)
        else:
            os.unlink(ufile_bookmarks)

    return user


def readAllUsers(o_out, i_full):
    users_dir = os.path.join(rulib.CGRU_LOCATION, 'users')
    try:
        listdir = os.listdir(users_dir)
    except:
        o_out['error'] = 'Can`t open users folder.'
        o_out['info'] = '%s' % traceback.format_exc()
        return;

    users = dict()
    for entry in listdir:
        if not os.path.isdir(os.path.join(users_dir, entry)):
            continue;

        user = readUser(entry, i_full);
        if user is not None:
            users[user['id']] = user

    return users


def writeUser(i_user, i_full):
    uid = i_user['id']
    udir = '%s/users/%s' % (rulib.CGRU_LOCATION, uid)
    ufile_main = '%s/%s.json' % (udir, uid)
    ufile_news = '%s/%s-news.json' % (udir, uid)
    ufile_bookmarks = '%s/%s-bookmarks.json' % (udir, uid)

    if not os.path.isdir(udir):
        try:
            os.makedirs(udir)
        except:
            print('writeUser: Unable create user folder: ' + udir)
            print('%s' % traceback.format_exc())
            return False

    user = dict()
    news = None
    bookmarks = None

    for key in i_user:
        if key == 'news':
            news = i_user['news']
        elif key == 'bookmarks':
            bookmarks = i_user['bookmarks']
        else:
            user[key] = i_user[key]

    if not writeObj(ufile_main, user):
        return False

    if not i_full:
        return True

    if news and len(news):
        writeObj(ufile_news, {'news': news})
    elif os.path.isfile(ufile_news):
        os.unlink(ufile_news)

    if bookmarks and len(bookmarks):
        writeObj(ufile_bookmarks, {'bookmarks': bookmarks})
    elif os.path.isfile(ufile_bookmarks):
        os.unlink(ufile_bookmarks)

    return True


def userChangedTasks(i_uid, i_tasks):
    if i_uid is None:
        i_uid = getCurUser()

    # Read acctivity from file, or initialize empty
    activity = dict()
    activity_file = '%s/users/%s/%s-activity.json' % (rulib.CGRU_LOCATION, i_uid, i_uid)
    if os.path.isfile(activity_file):
        activity = readObj(activity_file)

    curtime = getCurSeconds()
    for path in i_tasks:
        record = dict()
        # Get record by path, or create a new:
        if path in activity:
            record = activity.pop(path)
        else:
            record['ctime'] = curtime
        record['mtime'] = curtime
        record['task'] = i_tasks[path]

        # Insert activity as fisrt item (the latest time)
        activity = {path:record, **activity}

    # Keep maximum number of records:
    while len(activity) > 1000: #TODO
        activity.popitem()

    # Write activity file:
    writeObj(activity_file, activity)


def skipFile(i_filename):
    if i_filename in rulib.SKIPFILES:
        return True
    return False


def copyTemplate(i_uid, i_template, i_destination, i_names, o_out):
    if not os.path.isdir(i_template):
        o_out['error'] = 'Template folder does not exist.'
        return False
    if not os.path.isdir(i_destination):
        o_out['error'] = 'Destination folder does not exist.'
        return False
    if not type(i_names) is list:
        o_out['error'] = 'New names parameter should be list.'
        return False
    if len(i_names) == 0:
        o_out['error'] = 'New names parameter is an empty list.'
        return False
    if i_uid is None:
        i_uid = getCurUser()

    obj = dict()
    obj['cuser'] = i_uid
    obj['ctime'] = getCurSeconds()

    o_out['copies'] = []
    for name in i_names:
        copy = dict()

        dest = os.path.normpath(os.path.join(i_destination, name))
        copy['dest'] = dest
        if os.path.isdir(dest):
            copy['exist'] = True
        else:
            try:
               shutil.copytree(i_template, dest)
            except PermissionError:
                copy['error'] = 'Permission denied: %s' % dest
                continue
            except:
                copy['error'] = '%s' % traceback.format_exc()
                continue

            try:
                cfile = os.path.join(dest, rulib.RUFOLDER, 'location.json')
                cfileDir = os.path.dirname(cfile)
                if not os.path.isdir(cfileDir):
                    os.mkdir(cfileDir)
                writeObj(cfile, obj)
            except:
                copy['error'] = 'Unable to write rules file.'
                copy['info'] = '%s' % traceback.format_exc()

        o_out['copies'].append(copy)

    return True

def walkDir(i_admin, i_recv, i_dir, o_out, i_depth):
    if i_depth > i_recv['depth']:
        return

    if not os.path.isdir(i_dir):
        o_out['error'] = 'No such folder.'
        return

    rufolder = rulib.RUFOLDER
    if 'rufolder' in i_recv:
        rufolder = i_recv['rufolder']
    rufiles = None
    if 'rufiles' in i_recv:
        rufiles = i_recv['rufiles']
    lookahead = None
    if 'lookahead' in i_recv:
        lookahead = i_recv['lookahead']

    access = False
    denied = True
    if i_admin is None or i_admin.htaccessPath(i_dir):
        access = True
        denied = False
    else:
        rufiles = ['rules']
        o_out['denied'] = True

    try:
        listdir = os.listdir(i_dir)
    except:
        return

    o_out['folders'] = []
    o_out['files'] = []

    walk_file = os.path.join(i_dir, rufolder, 'walk.json')
    walk = readObj(walk_file)

    for entry in listdir:
        if skipFile(entry):
            continue

        path = os.path.join(i_dir, entry)

        if access and not os.path.isdir(path):
            if os.path.isfile(path):
                file_info = dict()
                if walk and 'files' in walk and entry in walk['files']:
                    file_info = walk['files'][entry]
                file_info['name'] = os.fsencode(entry).decode('utf-8', 'surrogateescape')

                st = os.stat(path)
                file_info['size'] = st.st_size
                file_info['mtime'] = st.st_mtime
                file_info['space'] = st.st_blocks * 512

                if 'mediainfo' in i_recv and i_recv['mediainfo']:
                    if cgruutils.isMovieExt(entry):
                        obj = mediainfo.processMovie(path)
                        if obj and 'mediainfo' in obj:
                            for k in obj['mediainfo']:
                                file_info[k] = obj['mediainfo'][k]

                o_out['files'].append(file_info)
            continue

        if entry == rufolder and os.path.isdir(path):
            o_out['rufiles'] = []
            o_out['rules'] = dict()

            try:
                rulist = os.listdir(path)
            except:
                continue

            for ruentry in rulist:
                if ruentry in ['.','..']:
                    continue

                if access:
                    o_out['rufiles'].append(ruentry)

                # No rufiles specified to read objects from
                if rufiles is None:
                    continue

                # Ensure in '.json' extension
                name, ext = os.path.splitext(ruentry)
                if ext != '.json':
                    continue

                # Check that file is specified in rufiles array
                found = False
                for rufile in rufiles:
                    if ruentry.find(rufile) == 0:
                        found = True
                        break

                if not found:
                    continue

                # Read object from rufile
                out = dict()
                obj = readObj(os.path.join(path, ruentry), out, False)
                if obj is not None:
                    o_out['rules'][ruentry] = obj
                else:
                    err = dict()
                    if 'error' in out:
                        err['ruerror'] = out
                    else:
                        err['ruerror'] = dict()
                        err['ruerror']['error'] = 'Unable to read json object'
                    o_out['rules'][ruentry] = err

            continue

        if os.path.isfile(os.path.join(path, '.hidden')):
            if 'showhidden' not in i_recv or not i_recv['showhidden']:
                continue

        if denied:
            continue

        if i_admin is not None and i_admin.htaccessFolder(path) is False:
            continue

        folderObj = dict()

        if walk and 'folders' in walk and entry in walk['folders']:
            folderObj = walk['folders'][entry]

        folderObj['name'] = os.fsencode(entry).decode('utf-8', 'surrogateescape')
        folderObj['mtime'] = os.path.getmtime(path)

        if rufolder and lookahead:
            for sfile in lookahead:
                sfilepath = os.path.join(path, rufolder, sfile) + '.json'
                rulib.editobj.mergeObjs(folderObj, readObj(sfilepath))
            thumbpath = os.path.join(path, rufolder, 'thumbnail.jpg')
            if os.path.isfile(thumbpath):
                folderObj['thumbnail'] = True

        if i_depth < i_recv['depth']:
            walkDir(i_admin, i_recv, path, folderObj, i_depth + 1)

        o_out['folders'].append(folderObj)


def upload(i_env, o_out):
    fieldStorage = cgi.FieldStorage(fp=i_env['wsgi.input'], environ=i_env, keep_blank_values=True)
    if 'upload_path' not in fieldStorage or 'upload_file' not in fieldStorage:
        o_out['error'] = 'Upload form should contain upload_path and upload_file fields.'
        return

    path = fieldStorage['upload_path'].value
    dirname = os.path.dirname(path)

    # Create a download directory
    if not os.path.isdir(os.fsdecode(dirname.encode())):
        try:
            os.makedirs(os.fsdecode(dirname.encode()))
        except:
            o_out['error'] = 'Unable to crearte upload directory: %s' % dirname
            o_out['info'] = '%s' % traceback.format_exc()
            return
        o_out['info'] = 'Upload directory created: %s' % dirname

    # If file exists add a nubmer
    basename, ext = os.path.splitext(os.path.basename(path))
    i = 0
    while os.path.isfile(os.fsdecode(path.encode())):
        i += 1
        path = os.path.join(dirname, '%s-%d' % (basename, i)) + ext

    file = None
    try:
        file = open(os.fsdecode(path.encode()), 'wb')
    except:
        o_out['error'] = 'Unable to open file: %s' % path
        o_out['info'] = '%s' % traceback.format_exc()
        return

    while 1:
        data = fieldStorage['upload_file'].file.read(1024)
        if not data:
            break
        file.write(data)

    o_out['upload'] = dict()
    o_out['upload']['path'] = path

