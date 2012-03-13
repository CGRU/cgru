menubar = nuke.menu('Nuke')
cgru_menu = menubar.addMenu('&CGRU')

cgru_menu.addCommand('&Afanasy Node', 'nuke.createNode("afanasy")', 'F10')
cgru_menu.addCommand('&Render Selected...', 'afanasy.render()', 'F11')

cgru_menu.addCommand("-", "", "")

cgru_menu.addCommand('Afanasy &Help', 'cgru.docsAfanasy()')
cgru_menu.addCommand('&Documentation', 'cgru.docsNuke()')
