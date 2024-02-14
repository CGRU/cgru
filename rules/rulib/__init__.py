import os
import sys
import traceback

CGRU_LOCATION = None
CGRU_VERSION = None

FILE_MAX_LENGTH = 3000000

HT_ACCESS_FILE_NAME = '.htaccess'
HT_GROUPS_FILE_NAME = '.htgroups'
HT_DIGEST_FILE_NAME = '.htdigest'

RUFOLDER = '.rules'
RECENT_FILENAME = 'recent.json'

RULES_TOP = dict()

ROOT = None

SKIPFILES = ['.', '..', HT_ACCESS_FILE_NAME, HT_GROUPS_FILE_NAME, HT_DIGEST_FILE_NAME]

GUESTCANCREATE = ['status.json', 'comments.json']
GUESTCANEDIT = ['comments.json']

if len(RULES_TOP) == 0:
    CGRU_LOCATION = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

    os.environ['CGRU_LOCATION'] = CGRU_LOCATION
    os.environ['AF_ROOT'] = os.path.join(CGRU_LOCATION, 'afanasy')

    sys.path.append(os.path.join(CGRU_LOCATION, 'lib', 'python'))
    sys.path.append(os.path.join(CGRU_LOCATION, 'afanasy', 'python'))


from . import comments
from . import editobj
from . import functions
from . import news
from . import search
from . import status


if len(RULES_TOP) == 0:
    CGRU_VERSION = functions.fileRead(os.path.join(CGRU_LOCATION,'version.txt'))
    os.environ['CGRU_VERSION'] = CGRU_VERSION

    RULES_TOP = functions.getRulesUno(CGRU_LOCATION,'rules')

    if 'root' in RULES_TOP:
        try:
            ROOT = os.readlink(os.path.join(CGRU_LOCATION, RULES_TOP['root']))
        except:
            print('%s' % traceback.format_exc())
            ROOT = os.path.join(CGRU_LOCATION, RULES_TOP['root'])


def setStatus(paths=None, uid=None, name=None, tags=None, tags_keep=None, artists=None, artists_keep=None, flags=None, flags_keep=None, progress=None, annotation=None, color=None, nonews=False, out=None):
    if out is None:
        out = dict()
    if paths is None or paths == []:
        paths = [None]

    statuses = []
    some_progress_changed = False

    for path in paths:
        st = status.Status(uid, path)
        st.set(tags=tags, tags_keep=tags_keep, artists=artists, artists_keep=artists_keep, flags=flags, flags_keep=flags_keep, progress=progress, annotation=annotation, color=color, out=out)
        if 'error' in out:
            break
        if st.progress_changed:
            some_progress_changed = True
        statuses.append(st)

    # News & Bookmarks:
    # At first we should emit news,
    # as some temporary could be added for news.
    # For example task.changed = true
    news.statusesChanged(statuses, out, nonews)

    out['statuses'] = []
    _out = dict()
    for st in statuses:
        st.save(_out)
        out['statuses'].append({"path":st.path,"status":st.data})

    if some_progress_changed:
        progresses = dict()
        progresses[st.path] = st.data['progress']
        status.updateUpperProgresses(os.path.dirname(st.path), progresses, out)

    return out


def setTask(paths=None, uid=None, name=None, tags=None, artists=None, flags=None, progress=None, annotation=None, deleted=None, nonews=False, out=None):
    if out is None:
        out = dict()
    if paths is None or paths == []:
        paths = [None]

    statuses = []
    tasks = dict()
    some_progress_changed = False

    for path in paths:
        st = status.Status(uid, path)
        task = st.setTask(name=name, tags=tags, artists=artists, flags=flags, progress=progress, annotation=annotation, deleted=deleted, out=out)
        if task is None or 'error' in out:
            break
        if st.progress_changed:
            some_progress_changed = True
        statuses.append(st)
        tasks[path] = task

    functions.userChangedTasks(uid, tasks)

    # News & Bookmarks:
    # At first we should emit news,
    # as some temporary could be added for news.
    # For example task.changed = true
    news.statusesChanged(statuses, out, nonews)

    out['statuses'] = []
    _out = dict()
    for st in statuses:
        st.save(_out)
        out['statuses'].append({"path":st.path,"status":st.data})

    if st.progress_changed:
        progresses = dict()
        progresses[st.path] = st.data['progress']
        status.updateUpperProgresses(os.path.dirname(st.path), progresses, out)

    return out

def setComment(paths=None, uid=None, ctype=None, text=None, tags=None, duration=None, color=None, uploads=None, deleted=False, nonews=None, out=None, key=None):
    if out is None:
        out = dict()
    if paths is None or paths == []:
        paths = [None]
 
    path_cdata = dict()
    for path in paths:
        cms = comments.Comments(uid, path)

        _out = dict()
        cdata = cms.add(ctype=ctype, text=text, tags=tags, duration=duration, color=color, uploads=uploads, deleted=deleted, out=_out, key=key)
        if 'error' in _out:
            out['error'] = _out['error']
            return out
        if cdata is None:
            return

        path_cdata[path] = cdata
        _out = dict()
        cms.save(_out)

        if 'error' in _out:
            out['error'] = _out['error']
            return out

    news.commentsChanged(path_cdata, uid=uid, out=out, nonews=nonews)

    out['comments'] = path_cdata

    return out

