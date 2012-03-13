foldersTree = gFld("<b>CGRU</b>", "", "../main.html")

insDoc(foldersTree, gLnk("Installation (RUS)", "", "../_cgru/installation.html"))
insDoc(foldersTree, gLnk("Setup", "", "../setup.html"))
insDoc(foldersTree, gLnk("Afanasy", "", "../../../../afanasy/plugins/maya/doc/afanasy.html"))
insDoc(foldersTree, gLnk("Features", "", "../_cgru/features.html"))

modeling_ = insFld(foldersTree, gFld("Modeling", "", ""))
animation_ = insFld(foldersTree, gFld("Animation", "", ""))
interface_ = insFld(foldersTree, gFld("Interface", "", ""))
rendering_ = insFld(foldersTree, gFld("Rendering", "", ""))
miscellaneous_ = insFld(foldersTree, gFld("Miscellaneous", "", ""))
develop_ = insFld(foldersTree, gFld("Develop", "", ""))

insDoc(foldersTree, gLnk("Hotkeys", "", "../_cgru/hotkeys.html"))
