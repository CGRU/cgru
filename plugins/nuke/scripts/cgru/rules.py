# -*- coding: utf-8 -*-
import os

import cgrules

import nuke


def rulesOpenShot():
	path = nuke.root().name()
	print('Scene = ' + path)

	cgrules.openShot( path, True)

