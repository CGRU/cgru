# -*- coding: utf-8 -*-
import os

bl_info = {
    "name": "CGRU Tools",
    "author": "Timur Hairulin, Paul Geraskin, Sylvain Maziere",
    "version": (1, 0, 0),
    "blender": (2, 7, 1),
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
    bpy.utils.register_module(__package__)
    bpy.types.Scene.cgru = bpy.props.PointerProperty(
        type=properties.CGRUProperties,
        name='CGRU Settings',
        description='CGRU Settings'
    )

    prefs = bpy.context.user_preferences.addons[__name__].preferences
    if "CGRU_LOCATION" in os.environ:
        prefs.cgru_location = os.environ["CGRU_LOCATION"]
    else:
        utils.add_cgru_module_to_syspath(prefs.cgru_location)


def unregister():
    del bpy.types.Scene.cgru
    bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
    register()
