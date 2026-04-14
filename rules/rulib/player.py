import os

import rulib


'''

python3 -c 'import sys; sys.path.append("/data/cgru/rules"); import rulib; print(rulib.player.playerInit(path="CG_PROJECT/SCENES/A_SCENE/A_SHOT_01//RESULT/JPG/v002"));'

'''

def playerInit(path=None, out=None):
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


    save_path = os.path.dirname(abspath) + '/.rules/' + os.path.basename(abspath) + '.player'
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

