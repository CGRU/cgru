from . import editobj
from . import functions

FILE_MAX_LENGTH = 3000000

HT_ACCESS_FILE_NAME = '.htaccess'
HT_GROUPS_FILE_NAME = '.htgroups'
HT_DIGEST_FILE_NAME = '.htdigest'

RUFOLDER = '.rules'
RECENT_FILENAME = 'recent.json'

SKIPFILES = ['.', '..', HT_ACCESS_FILE_NAME, HT_GROUPS_FILE_NAME, HT_DIGEST_FILE_NAME]

GUESTCANCREATE = ['status.json', 'comments.json']
GUESTCANEDIT = ['comments.json']
