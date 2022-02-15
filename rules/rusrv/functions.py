import cgi
import json
import os
import sys
import time
import traceback

from rusrv import admin
from rusrv import environ
from rusrv import editobj


def randMD5():
    hashlib = __import__('hashlib', globals(), locals(), [])
    random = __import__('random', globals(), locals(), [])
    return hashlib.md5(str(random.random()).encode()).hexdigest()


def fileRead(i_file, i_lock = True, i_verbose = False):
    try:
        f = open(i_file, mode='r', encoding='utf-8')
    except:
        return None

    data = f.read(environ.FILE_MAX_LENGTH)
    f.close()

    if i_verbose:
        print('fileRead: Read %d bytes from %s.' % (len(data), i_file))

    return data

def fileWrite(i_file, i_data, i_lock = True, i_verbose = False):
    tmp_name = ('%s-%s') % (i_file, os.getpid())
    try:
        f = open(tmp_name, mode='w', encoding='utf-8')
    except:
        print('fileWrite: Unable open for writing: ' + tmp_name)
        print('%s' % traceback.format_exc())
        return False

    #if ($i_lock) _flock_($fHandle, LOCK_EX)
    f.write(i_data)
    #if ($i_lock) _flock_($fHandle, LOCK_UN)
    f.close()

    os.rename(tmp_name, i_file)

    if i_verbose:
        print('fileWrite: Written %d bytes to: %s' % (len(i_data), i_file))

    return True


def readObj(i_file, o_out = None, i_lock = True):
    if not os.path.isfile(i_file):
        if o_out:
            o_out['error'] = 'No such file %s' % i_file
        return

    data = fileRead(i_file, i_lock)
    obj = None
    if data:
        obj = json.loads(data)
        return obj

    if o_out:
        o_out['error'] = 'Unable to read file %s' % i_file

    return


def writeObj(i_file, i_obj, i_lock = True, i_verbose = False):
    if fileWrite(i_file, json.dumps(i_obj, indent='\t', sort_keys=True), i_lock, i_verbose):
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
    ufile_main = 'users/%s/%s.json' % (i_uid, i_uid)
    ufile_news = 'users/%s/%s-news.json' % (i_uid, i_uid)
    ufile_bookmarks = 'users/%s/%s-bookmarks.json' % (i_uid, i_uid)

    if not os.path.isfile(ufile_main):
        return None

    user = readObj(ufile_main)
    if user is None:
        return None

    if not i_full:
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
    try:
        listdir = os.listdir('users')
    except:
        o_out['error'] = 'Can`t open users folder.'
        o_out['info'] = '%s' % traceback.format_exc()
        return;

    users = dict()
    for entry in listdir:
        if not os.path.isdir(os.path.join('users', entry)):
            continue;

        user = readUser(entry, i_full);
        if user is not None:
            users[user['id']] = user

    return users


def processUser(i_arg, o_out):
    if not os.path.isdir('users'):
        os.mkdir('users')

    if environ.USER_ID is None:
        return

    user = readUser(environ.USER_ID, True)
    if user is None:
        user = dict()

    if 'error' in user:
        o_out['error'] = user['error']

    user['rtime'] = int(time.time())
    if not 'id'       in user: user['id']       = environ.USER_ID
    if not 'channels' in user: user['channels'] = []
    if not 'news'     in user: user['news']     = []
    if not 'ctime'    in user: user['ctime']    = user['rtime']

    processUserIP(i_arg, user)

    # Delete nulls from some arrays:
    arrays = ['news', 'bookmarks', 'channels']
    for arr in arrays:
        if arr in user and user[arr] is not None:
            user[arr] = list(filter(lambda a: a != None, user[arr]))

    if not writeUser(user, False):
        o_out['error'] = 'Can`t write current user'
        return

    o_out['user'] = user;


def processUserIP(i_args, o_user):
    ip = environ.REMOTE_ADDR

    if not 'ips' in o_user:
        o_user['ips'] = []

    # Remove other enties from this IP:
    o_user['ips'] = list(filter(lambda a: a['ip'] != ip, o_user['ips']))

    entry = dict()
    entry['ip'] = ip
    entry['time'] = int(time.time())
    if 'url' in i_args:
        entry['url'] = i_args['url']

    # Insest and entry in the beginning
    o_user['ips'].insert(0, entry)

    # Limit entries count 
    o_user['ips'] = o_user['ips'][:10]


def writeUser(i_user, i_full):
    uid = i_user['id']
    ufile_main = 'users/%s/%s.json' % (uid, uid)
    ufile_news = 'users/%s/%s-news.json' % (uid, uid)
    ufile_bookmarks = 'users/%s/%s-bookmarks.json' % (uid, uid)

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


def skipFile(i_filename):
    if i_filename in environ.SKIPFILES:
        return True
    return False


def walkDir(i_recv, i_dir, o_out, i_depth):
    if i_depth > i_recv['depth']:
        return

    if not os.path.isdir(i_dir):
        o_out['error'] = 'No such folder.'
        return

    rufolder = None
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
    if admin.htaccessPath(i_dir):
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
                obj = readObj(os.path.join(path, ruentry), None, False)
                if obj:
                    o_out['rules'][ruentry] = obj

            continue

        if os.path.isfile(os.path.join(path, '.hidden')):
            if 'showhidden' not in i_recv or not i_recv['showhidden']:
                continue

        if denied:
            continue

        if admin.htaccessFolder(path) is False:
            continue

        folderObj = dict()

        if walk and 'folders' in walk and entry in walk['folders']:
            folderObj = walk['folders'][entry]

        folderObj['name'] = os.fsencode(entry).decode('utf-8', 'surrogateescape')
        folderObj['mtime'] = os.path.getmtime(path)

        if rufolder and lookahead:
            for sfile in lookahead:
                sfilepath = os.path.join(path, rufolder, sfile) + '.json'
                editobj.mergeObjs(folderObj, readObj(sfilepath))

        if i_depth < i_recv['depth']:
            walkDir(i_recv, path, folderObj, i_depth + 1)

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

