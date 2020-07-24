# -*- coding: utf-8 -*-
import os

bl_info = {
    "name": "CGRU Tools",
    "author": "Timur Hairulin, Paul Geraskin, Sylvain Maziere",
    "version": (1, 0, 0),
    "blender": (2, 80, 0),
    "location": "Properties > Render > Afanasy",
    "description": "CGRU Tools",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Tools"
}


if "bpy" in locals():
    import imp
    imp.reload(addon_prefs)
    imp.reload(properties)
    imp.reload(ui)
    imp.reload(operators)
    imp.reload(utils)
else:
    from . import addon_prefs
    from . import properties
    from . import ui
    from . import operators
    from . import utils

import bpy

def register():
    from bpy.types import Scene

    bpy.utils.register_class(addon_prefs.CGRUAddonPreferences)
    bpy.utils.register_class(properties.CGRUProperties)
    bpy.utils.register_class(ui.RENDER_PT_Afanasy)
    bpy.utils.register_class(operators.CGRU_Browse)
    bpy.utils.register_class(operators.CGRU_Submit)
    Scene.cgru = bpy.props.PointerProperty(type=properties.CGRUProperties)

    prefs = bpy.context.preferences.addons[__name__].preferences
    location=os.environ.get("CGRU_LOCATION")
    if location is not None:
        prefs.cgru_location = location
    else:
        utils.add_cgru_module_to_syspath(prefs.cgru_location)

def unregister():
    del bpy.types.Scene.cgru

    bpy.utils.unregister_class(addon_prefs.CGRUAddonPreferences)
    bpy.utils.unregister_class(properties.CGRUProperties)
    bpy.utils.unregister_class(ui.RENDER_PT_Afanasy)
    bpy.utils.unregister_class(operators.CGRU_Browse)
    bpy.utils.unregister_class(operators.CGRU_Submit)

if __name__ == "__main__":
    register()
