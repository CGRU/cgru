# CGRU init GUI script

import NatronGui

NatronGui.natron.addMenuCommand('CGRU/Afanasy Node','reload(afanasy);afanasy.createNode(app)',QtCore.Qt.Key.Key_F10,QtCore.Qt.KeyboardModifier.NoModifier)

NatronGui.natron.addMenuCommand('CGRU/Render Selected...','reload(afanasy);afanasy.renderSelected(app)',QtCore.Qt.Key.Key_F11,QtCore.Qt.KeyboardModifier.NoModifier)
