# -*- coding: utf-8 -*-

import os

SoundRef = 'REF/sound.wav'

SoundFileExt = ['.flac','.mp3','.mp4','.mov','.wav']

def findSoundRef(i_folder):
    if not os.path.isdir(i_folder):
        print('Sound folder not found: "%s"' % i_folder)
        return None

    shot = os.path.basename(os.getcwd())

    files_shot = []
    files_other = []

    for afile in os.listdir(i_folder):
        afile = os.path.join(i_folder, afile)
        if not os.path.isfile(afile):
            continue

        name, ext = os.path.splitext(afile)
        if ext not in SoundFileExt:
            continue

        if afile == SoundRef:
            return afile

        if os.path.basename(afile).find(shot) == 0:
            files_shot.append(afile)
        else:
            files_other.append(afile)

    files_shot.sort()
    files_other.sort()

    if len(files_shot):
        return files_shot[-1]

    if len(files_other):
        return files_other[-1]

    return None

