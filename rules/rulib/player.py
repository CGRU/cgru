import base64
import os

import rulib


'''

python3 -c 'import sys; sys.path.append("/data/cgru/rules"); import rulib; print(rulib.player.init(path="/CG_PROJECT/SCENES/A_SCENE/A_SHOT_01//RESULT/JPG/v002"));'

python3 -c 'import sys; sys.path.append("/data/cgru/rules"); import rulib; print(rulib.player.save({"path":"/CG_PROJECT/SCENES/A_SCENE/A_SHOT_01//RESULT/JPG/v002","comments":{"asdf":{"text":"fghddfghg"}},"jpegs":[{"name":"asdf","data":"qwer"}]}));'

'''

def getSavePath(i_path):
    return os.path.dirname(i_path) + '/.rules/' + os.path.basename(i_path) + '.player'
def getJPEGPath(i_path, i_name):
    path = getSavePath(i_path)
    return path + '/' + i_name + '.jpg'
def getPNGPath(i_path, i_name):
    path = getSavePath(i_path)
    return path + '/canvas/' + i_name + '.png'

def init(path=None, out=None):
    if out is None:
        out = dict()

    abspath = os.getcwd()
    if path:
        abspath = rulib.functions.getAbsPath(path)

    if not os.path.isdir(abspath):
        out['error'] = 'Folder %s does not exist.' % path
        return out

    out['path'] = path
    out['images'] = []
    for name in sorted(os.listdir(abspath)):
        if name[0] == '.':
            continue
        img = dict()
        img['name'] = name
        img['size'] = os.path.getsize(os.path.join(abspath, name))
        out['images'].append(img)


    sound_path = path.split('//')[0] + '/REF/sound.wav'
    print(sound_path)
    if os.path.isfile(rulib.functions.getAbsPath(sound_path)):
        out['sound'] = sound_path


    save_path = getSavePath(abspath)
    if os.path.isdir(save_path):

        data = rulib.functions.readObj(os.path.join(save_path, 'data.json'))
        if data:
            for key in data:
                out[key] = data[key]

        canvas_path = os.path.join(save_path, 'canvas')
        if os.path.isdir(canvas_path):
            out['canvas'] = []
            for name in os.listdir(canvas_path):
                out['canvas'].append(name)

        painted_path = os.path.join(save_path, 'painted')
        if os.path.isdir(painted_path):
            out['painted'] = []
            for name in os.listdir(painted_path):
                out['painted'].append(name)

    return out


def save(i_obj, uid=None, out=None):
    if out is None:
        out = dict()
    if uid is None:
        uid = rulib.functions.getCurUser()

    if not os.path.isdir(rulib.functions.getAbsPath(i_obj['path'])):
        out['error'] = 'Folder %s does not exist.' % path
        return out

    out['path'] = i_obj['path']

    savePNGs(i_obj, out);
    saveJPEGs(i_obj, out);
    saveComments(i_obj, uid, out);

    return out

def savePNGs(i_obj, out):
    if not 'pngs' in i_obj:
        return

    pngs = i_obj['pngs']

    if len(pngs) == 0:
        return

    out['pngs'] = []

    for png in pngs:
        path = getPNGPath(i_obj['path'], png['name'])
        path = rulib.functions.getAbsPath(path)
        dirname = os.path.dirname(path)

        if not os.path.isdir(dirname):
            try:
                os.makedirs(dirname, mode=0o777)
            except:
                out['error'] = '%s' % traceback.format_exc()
                return

        if not rulib.functions.fileWrite(path, base64.b64decode(png['data'])):
            out['error'] = 'Unable to open save file: ' + filename
        else:
            out['pngs'].append(png['name'])

def saveJPEGs(i_obj, out):
    if not 'jpegs' in i_obj:
        return

    jpegs = i_obj['jpegs']

    if len(jpegs) == 0:
        return

    for jpg in jpegs:
        path = getJPEGPath(i_obj['path'], jpg['name'])
        path = rulib.functions.getAbsPath(path)
        dirname = os.path.dirname(path)

        if not os.path.isdir(dirname):
            try:
                os.makedirs(dirname, mode=0o777)
            except:
                out['error'] = '%s' % traceback.format_exc()
                return

        if not rulib.functions.fileWrite(path, base64.b64decode(jpg['data'])):
            o_out['error'] = 'Unable to open save file: ' + filename

def saveComments(i_obj, uid, out):
    if not 'comments' in i_obj:
        return

    icomments = i_obj['comments']

    if len(icomments) == 0:
        return

    comments_path = getSavePath(i_obj['path']) + '/data.json'
    comments_path = rulib.functions.getAbsPath(comments_path)
    comments_obj = rulib.functions.readObj(comments_path)
    if comments_obj is None:
        comments_obj = dict()
    if not 'comments' in comments_obj:
        comments_obj['comments'] = dict()

    text = '<div class="player"><a target="_blank" class="player_title" href="%s">Player comments:</a>' % ('player.html#' + i_obj['path'])
    for name in icomments:
        cm = icomments[name]
        if 'cuser' in cm:
            cm['muser'] = uid
            cm['mtime'] = rulib.functions.getCurSeconds()
        else:
            cm['cuser'] = uid
            cm['ctime'] = rulib.functions.getCurSeconds()
        comments_obj['comments'][name] = cm

        img_path = getJPEGPath(i_obj['path'], name)
        img_path = rulib.RULES_TOP['root'] + '/' + img_path

        text += '<p class="player_comment">'
        text += '<a target="_blank" href="%s" class="player_name_link"><div class="player_name">%s</div></a>' % (img_path, name)
        text += '<a target="_blank" href="%s" class="player_img_link"><img class="player_img" src="%s"></a>' % (img_path, img_path)
        text += '<div class="player_comment_text">' + cm['text'] + '</div>';
        text += '</p>'
    text += '</div>'

    rulib.functions.writeObj(comments_path, comments_obj)

    rules_path = i_obj['path'].split('//')[0]
    rulib.setComment(paths=[rules_path], uid=uid, ctype=None, text=text, tags=None, duration=None, color=None, uploads=None, deleted=False, nonews=None, out=out, key=None)

