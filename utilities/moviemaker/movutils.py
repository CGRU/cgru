# -*- coding: utf-8 -*-

import os
import subprocess
import sys

SoundRef = 'REF/sound.wav'

SoundFileExt = ['.flac','.mp3','.mp4','.mov','.wav']

def hasAudio(i_file):
    if not os.path.isfile(i_file):
        return False

    cmd = 'ffprobe'
    cmd += ' -i "%s"' % i_file
    cmd += ' -show_streams -select_streams a -loglevel error'

    data = subprocess.check_output(cmd, shell=True)
    if len(data) == 0:
        return False

    return True

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

        if not hasAudio(afile):
            continue

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

