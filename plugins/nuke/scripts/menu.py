menubar = nuke.menu('Nuke')
cgru_menu = menubar.addMenu('CGRU')

cgru_menu.addCommand('Afanasy Node', 'nuke.createNode("afanasy")', 'F10')
cgru_menu.addCommand('Dailies Node', 'nuke.createNode("cgru_dailies")')
cgru_menu.addCommand('Render Selected...', 'afanasy.render()', 'F11')

cgru_submenu = cgru_menu.addMenu("Paths")
cgru_submenu.addCommand('Cut Working Directory', 'cgru.pathcurdir.cut()')
cgru_submenu.addCommand('Add Working Directory', 'cgru.pathcurdir.add()')
cgru_submenu.addCommand("-", "", "")
cgru_submenu.addCommand('Save Scene To Server Paths', 'cgru.pmSaveToServer()')
cgru_submenu.addCommand('Open Scene From Server Paths', 'cgru.pmOpenFromServer()')

cgru_menu.addCommand("-", "", "")

#cgru_menu.addCommand('Afanasy Help', 'cgru.docsAfanasy()')
cgru_menu.addCommand('Documentation', 'cgru.docsNuke()')
