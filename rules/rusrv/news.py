import os
import time

from rusrv import environ
from rusrv import functions

def makenews(i_args, io_users, o_out):
    news = i_args['news']

    # Ensure that news has a path:
    if not 'path' in news:
        o_out['error'] = 'News can`t be without a path.'
        return False

    path = news['path']

    # Path should not be an empty string:
    if len(path) == 0:
        o_out['error'] = 'Path is an empty string.'
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
        rfile = i_args['root'] + path + '/' + i_args['rufolder'] + '/' + i_args['recent_file']
        rarray = functions.readObj(rfile)
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

            while len(rarray) >= i_args['recent_max']:
                del rarray[-1]

        # Add new recent:
        rarray.insert(0, news)

        # Save recent:
        if not os.path.isdir(os.path.dirname(rfile)):
            os.makedirs(os.path.dirname(rfile))
        functions.writeObj(rfile, rarray)

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
    if 'ignore_own' in i_args and i_args['ignore_own']:
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
        limit = 99
        if 'limit' in i_args and i_args['limit'] > 0:
            limit = i_args['limit']
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

    return changed_users


def makebookmarks(i_bm, io_users, o_out):
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

            if user['bookmarks'][i]['path'] == i_bm['path']:
                bm_index = i
                break

        # Bookmark with the same path does not exist:
        if bm_index == -1:
            # Check whether the bookmark is needed:
            if not isUserAssignedInStatus(user, i_bm):
                continue

            # Initialize parameters:
            i_bm['cuser'] = environ.USER_ID
            i_bm['ctime'] = int(time.time())
        else:
            # Bookmark exists
            # Copy creation parameters
            i_bm['cuser'] = user['bookmarks'][bm_index]['cuser']
            i_bm['ctime'] = user['bookmarks'][bm_index]['ctime']

            # Delete existing bookmark,
            # no updating, just new will be created
            del user['bookmarks'][i]

        i_bm['muser'] = environ.USER_ID
        i_bm['mtime'] = int(time.time())

        user['bookmarks'].append(i_bm)
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

            # Below situations for not done tasks only
            if not 'progress' in task or task['progress'] >= 100:
                continue

            # If status head changed, all tasks users should receive news (if task is not done)
            if 'changed' in status and status['changed']:
                return True

            # If it is a news on change something, but not status (body, comments),
            # All tasks users should receive news (if task is not done)
            if 'title' in i_obj and i_obj['title'] != 'status':
                return True

    return False

