import os
import webbrowser

import cgruconfig
import cgruutils

def show(i_path=None):

    url = cgruconfig.VARS['docs_url']

    if i_path is not None:
        i_path.rstrip('/')
        url = '%s/%s' % (url, i_path)

    cgruutils.webbrowse( url)

def showSoftware( i_soft):
    return show('software/%s' % i_soft)

def showForum( i_forum = None):

    forums = dict()
    forums['nuke']  = 20
    forums['watch'] = 16

    url = cgruconfig.VARS['forum_url']

    if i_forum is not None and i_forum in forums:
        url += '/viewforum.php?f=%d' % forums[i_forum]

    cgruutils.webbrowse( url)

