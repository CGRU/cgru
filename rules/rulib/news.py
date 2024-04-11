import os
import time

import rulib

def makeNewsAndBookmarks(i_args, i_uid, out=dict(), nonews=False):
    # Read all users:
    users = rulib.functions.readAllUsers(out, True)
    if 'error' in out:
        return

    if len(users) == 0:
        out['error'] = 'No users found.'
        return

    users_changed = []

    bookmarks = []
    # TODO
    i_news = []
    if 'news' in i_args:
        i_news = i_args['news']
    elif 'news_requests' in i_args:
        i_news = i_args['news_requests']
    for news in i_news:
        if 'news' in news:
            news = news['news']

        bookmarks.append({'status':news['status'],'path':news['path']})

        if nonews:
            continue

        ids = makeNewsUno(news, users, i_uid, out)
        if 'error' in out:
            return

        if ids is not None:
            for id in ids:
                if not id in users_changed:
                    users_changed.append(id)

    if 'bookmarks' in i_args:
        bookmarks = i_args['bookmarks']
    for bm in bookmarks:
        ids = makeBookmarks(i_uid, bm, users, out)
        if 'error' in out:
            return

        if ids is not None:
            for id in ids:
                if not id in users_changed:
                    users_changed.append(id)

    # Write changed users:
    for id in users_changed:
        rulib.functions.writeUser(users[id], True)

    out['users_changed'] = users_changed

def makeNewsUno(i_args, io_users, i_uid, out):
    news = i_args

    # Ensure that news has a path:
    if not 'path' in news:
        out['error'] = 'News can`t be without a path.'
        return False

    path = news['path']

    # Path should not be an empty string:
    if len(path) == 0:
        out['error'] = 'Path is an empty string.'
        return False

    # Ensure that the first path character is '/':
    if path[0] != '/':
        path = '/' + path

    # Ensure that path last character is not '/' (if path is not just '/' root):
    if path != '/' and path[-1] == '/':
        path = path[:-1]

    # Process recent for current and each parent folder till root:
    for i in range(101):
        # Simple loop check:
        if i == 100:
            break

        # Get existing recent:
        rfile = rulib.functions.getRuFilePath(rulib.RECENT_FILENAME, path)
        rarray = rulib.functions.readObj(rfile)
        if rarray is None:
            rarray = []

        count = len(rarray)
        if len(rarray):
            if i:
                # Remove all news with the same path from all parent folders,
                # so folder has only one recent from each child:
                j = 0
                while j < count:
                    if rarray[j]['path'] == news['path']:
                        del rarray[j]
                        count = len(rarray)
                    else:
                        j += 1
            else:
                # Remove latest recent news if it is the same:
                if rarray[0]['path'] == news['path'] and rarray[0]['title'] == news['title'] and rarray[0]['user'] == news['user']:
                    del rarray[0]

            while len(rarray) >= rulib.RULES_TOP['news']['recent']:
                del rarray[-1]

        # Add new recent:
        rarray.insert(0, news)

        # Save recent:
        if not os.path.isdir(os.path.dirname(rfile)):
            try:
                os.makedirs(os.path.dirname(rfile))
            except PermissionError:
                out['info'] = 'Permission denied to make folder for recent: "%s"' % os.path.dirname(rfile)
                continue
            except:
                out['info'] = 'Unable to create folder for recent: "%s"' % traceback.format_exc()
                continue
        rulib.functions.writeObj(rfile, rarray)

        # Exit cycle if path is root:
        if len(path) == 0:
            break

        # Set path to parent folder:
        path_prev = path
        path = os.path.dirname(path)
        # Stop cycle if can't go to parent folder:
        if path == path_prev:
            break

        i += 1


    # Process users subsriptions:

    # User may be does not want to receive own news:
    ignore_own = False
    if i_uid in io_users and 'ignore_own' in io_users[i_uid] and io_users[i_uid]['ignore_own']:
        ignore_own = True

    # Get subscribed users:
    sub_users = []
    changed_users = []
    for id in io_users:
        user = io_users[id]
        # If this is news owner:
        if news['user'] == user['id']:
            # Store last news time:
            user['ntime'] = int(time.time())
            if not user['id'] in changed_users:
                changed_users.append(user['id'])

            # If user does not want to receive own news:
            if ignore_own:
                continue

        # If user is assigned, it should receive news:
        if isUserAssignedInStatus(user, news):
            if not user['id'] in sub_users:
                sub_users.append(user['id'])
            continue

        # Check user subscriptions:
        if 'channels' in user:
            for channel in user['channels']:
                if news['path'].find(channel['id']) == 0:
                    if filterPassed(user, news):
                        if not user['id'] in sub_users:
                            sub_users.append(user['id'])
                        break


    # Add news and write files:
    for id in sub_users:
        user = io_users[id]

        # Add uid to changed array:
        if not user['id'] in changed_users:
            changed_users.append(user['id'])

        # On empty news create an empty news array:
        if not 'news' in user or not isinstance(user['news'], list):
            user['news'] = []

        # Delete older news with the same path:
        i = 0
        while i < len(user['news']):
            if user['news'][i]['path'] == news['path']:
                del user['news'][i]
                break
            else:
                i += 1

        # Add news to the beginning of array:
        user['news'].insert(0, news)

        # Delete news above the limit:
        limit = rulib.RULES_TOP['news']['limit']
        if 'news_limit' in user and user['news_limit'] > 0:
            limit = user['news_limit']

        while len(user['news']) > limit:
            del user['news'][-1]

#TODO
#        # Send emails:
#        if (array_key_exists('email', $user) == false) continue;
#        if (array_key_exists('email_news', $user) == false) continue;
#        if ($user['email_news'] != true) continue;
#
#        $mail = array();
#        $mail['from_title'] = $i_args['email_from_title'];
#        $mail['address'] = $user['email'];
#        $mail['subject'] = $i_args['email_subject'];
#        $mail['body'] = $i_args['email_body'];
#
#        $out = array();
#        jsf_sendmail($mail, $out);

    out['users_subscribed'] = sub_users

    return changed_users


def makeBookmarks(i_user_id, i_bm, io_users, out):
    changed_users = []
    for id in io_users:
        user = io_users[id]
        if not 'bookmarks' in user:
            user['bookmarks'] = []

        # Try to find existing bookmark index with the same path:
        bm_index = -1
        for i in range(len(user['bookmarks'])):
            if user['bookmarks'][i] is None:
                continue

            if not 'path' in user['bookmarks'][i]:
                continue

            if user['bookmarks'][i]['path'] == i_bm['path']:
                bm_index = i
                break

        # Copy a dict, as we will edit it per user personally
        bm = i_bm.copy()

        # Bookmark with the same path does not exist:
        if bm_index == -1:
            # Check whether the bookmark is needed:
            if not isUserAssignedInStatus(user, bm):
                continue

            # Initialize parameters:
            bm['cuser'] = i_user_id
            bm['ctime'] = int(time.time())
        else:
            # Bookmark exists
            # Copy creation parameters
            bm['cuser'] = user['bookmarks'][bm_index]['cuser']
            bm['ctime'] = user['bookmarks'][bm_index]['ctime']
            if 'favourite' in user['bookmarks'][bm_index] and user['bookmarks'][bm_index]['favourite']:
                bm['favourite'] = True

            # Delete existing bookmark,
            # no updating, just new will be created
            del user['bookmarks'][i]

        bm['muser'] = i_user_id
        bm['mtime'] = int(time.time())

        user['bookmarks'].append(bm)
        changed_users.append(user['id'])

    return changed_users


def isUserAssignedInStatus(i_user, i_obj):
    if not 'status' in i_obj:
        return False

    status = i_obj['status']
    if not isinstance(status, dict):
        return False

    # Check if user is assigned in status
    if 'artists' in status:
        if i_user['id'] in status['artists']:
                return True

    # Check if user is assigned in some task
    if 'tasks' in status:
        for tname in status['tasks']:
            task = status['tasks'][tname]

            if 'deleted' in task and task['deleted']:
                continue
            if not 'artists' in task:
                continue
            if not i_user['id'] in task['artists']:
                continue

            if 'changed' in task and task['changed']:
                return True

            # Below situations for not done or omit tasks
            if not 'progress' in task or task['progress'] >= 100 or task['progress'] < 0:
                continue

            # If status head changed, all tasks users should receive news (if task is not done)
            if 'changed' in status and status['changed']:
                return True

            # If it is a news on change something, but not status (body, comments),
            # All tasks users should receive news (if task is not done)
            if 'title' in i_obj and i_obj['title'] != 'status':
                if 'tags' in i_obj:
                    # If news has tags, we should send it only to users with task contains news tag.
                    # Comments and reports can have tags.
                    found = False
                    for tag in i_obj['tags']:
                        if tag in task['tags']:
                            found = True
                            break
                    if not found:
                        continue
                return True

    return False


def filterPassed(i_user, i_obj):

    if not 'news_filter' in i_user:
        return True

    if not 'status' in i_obj:
        return True

    status = i_obj['status']
    if not isinstance(status, dict):
        return True

    # Find out wat was changed:
    changed = None
    if 'changed' in status and status['changed']:
        changed = status
    else:
        if 'tasks' in status:
            for tname in status['tasks']:
                task = status['tasks'][tname]

                if 'deleted' in task and task['deleted']:
                    continue

                if 'changed' in task and task['changed']:
                    changed = task

    if changed is None:
        return True

    news_filter = i_user['news_filter']
    #print(news_filter)
    #print(changed)

    if not checkIntersection(news_filter, 'tags_include', True, changed, 'tags', False):
        return False
    if not checkIntersection(news_filter, 'flags_include', True, changed, 'flags', False):
        return False
    if checkIntersection(news_filter, 'tags_exclude', False, changed, 'tags', False):
        return False
    if checkIntersection(news_filter, 'flags_exclude', False, changed, 'flags', False):
        return False

    return True


def checkIntersection(obj_a, key_a, no_a, obj_b, key_b, no_b):
    if not key_a in obj_a:
        return no_a
    list_a = obj_a[key_a]
    if len(list_a) == 0:
        return no_a

    if not key_b in obj_b:
        return no_b
    list_b = obj_b[key_b]
    if len(list_b) == 0:
        return no_b

    for i in list_a:
        if i in list_b:
            return True

    return False


# Remove not needed status fields:
def filterStatus(i_sdata):
    if i_sdata is None:
        return None
    sdata = dict()
    skip_keys = ['body']
    for key in i_sdata:
        if not key in skip_keys:
            sdata[key] = i_sdata[key]
    return sdata


def statusesChanged(i_statuses, out=dict(), nonews=False):
    news = []
    for status in i_statuses:
        news.append(createNews(title='status', path=status.path, uid=status.data['muser'], status=status.data))
    if len(news):
        makeNewsAndBookmarks({'news':news}, i_statuses[0].data['muser'], out=out, nonews=nonews)


def commentsChanged(i_path_cdata, uid=None, out=dict(), nonews=False):
    news = []
    title = 'comment'
    for path in i_path_cdata:
        cdata = i_path_cdata[path]
        sdata = rulib.status.getStatusData(path)
        if 'type' in cdata and cdata['type'] == 'report':
            title = 'report'
        tags = None
        if 'tags' in cdata and len(cdata['tags']):
            tags = cdata['tags']
        news.append(createNews(title=title, path=path, uid=uid, status=sdata, tags=tags))
    if len(news):
        makeNewsAndBookmarks({'news':news}, uid, out=out, nonews=nonews)


def createNews(title='news', uid=None, path=None, status=None, tags=None):
    if uid is None: uid = rulib.functions.getCurUser()
    if path is None: path = os.getcwd()
    if status is None: rulib.status.getStatusData(path)

    news = dict()
    news['title'] = title
    news['path'] = path
    news['user'] = uid
    news['status'] = filterStatus(status)
    news['time'] = rulib.functions.getCurSeconds()
    news['id'] = news['user'] + '_' + str(news['time']) + '_' + news['path']
    if tags is not None:
        news['tags'] = tags

    return news
