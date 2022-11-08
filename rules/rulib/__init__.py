import os

from . import editobj
from . import functions
from . import news
from . import search
from . import status

FILE_MAX_LENGTH = 3000000

HT_ACCESS_FILE_NAME = '.htaccess'
HT_GROUPS_FILE_NAME = '.htgroups'
HT_DIGEST_FILE_NAME = '.htdigest'

RUFOLDER = '.rules'
RECENT_FILENAME = 'recent.json'

RULES_TOP = dict()

SKIPFILES = ['.', '..', HT_ACCESS_FILE_NAME, HT_GROUPS_FILE_NAME, HT_DIGEST_FILE_NAME]

GUESTCANCREATE = ['status.json', 'comments.json']
GUESTCANEDIT = ['comments.json']

if len(RULES_TOP) == 0:
    cgru_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
    RULES_TOP = functions.getRulesUno(cgru_root,'rules')
