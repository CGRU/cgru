import os

import cgrudocs
import cgrules

# Add CGRU to main window menu:
cgru_menu = nuke.menu('Nuke').addMenu('CGRU')

if cgrules.hasRULES():
	cgru_menu.addCommand('RULES', 'cgru.rulesOpenShot()', 'F12')

cgru_menu.addCommand('Afanasy Node', 'nuke.createNode("afanasy")', 'F10')
cgru_menu.addCommand('Dailies Node', 'nuke.createNode("cgru_dailies")')
cgru_menu.addCommand('Render Selected...', 'cgru.render()', 'F11')

cgru_submenu = cgru_menu.addMenu("Paths")
cgru_submenu.addCommand('Cut Working Directory', 'cgru.pathcurdir.cut()')
cgru_submenu.addCommand('Add Working Directory', 'cgru.pathcurdir.add()')
cgru_submenu.addCommand("-", "", "")
cgru_submenu.addCommand('Open Translated', 'cgru.pmOpenTranslated()')

cgru_menu.addCommand("-", "", "")

cgru_menu.addCommand('Forum...',         'cgrudocs.showForum("nuke")')
cgru_menu.addCommand('Documentation...', 'cgrudocs.showSoftware("nuke")')

# Add afanasy gizmo to nodes:
icons = os.path.join(os.environ['CGRU_LOCATION'], 'icons')
nuke.menu('Nodes').addMenu('CGRU', icon=os.path.join(icons, 'keeper.png'))
nuke.menu('Nodes').addCommand('CGRU/afanasy', 'nuke.createNode("afanasy")',
							  icon=os.path.join(icons, 'afanasy.png'))
