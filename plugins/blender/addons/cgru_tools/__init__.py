# -*- coding: utf-8 -*-

bl_info = {
	"name": "CGRU Tools",
	"author": "Timur Hairulin, Paul Geraskin, Sylvain Maziere",
	"version": (1,0,0),
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
else:
	from . import addon_prefs
	from . import properties
	from . import ui
	from . import operators

import bpy

def register():
	bpy.utils.register_module(__name__)
	bpy.types.Scene.cgru = bpy.props.PointerProperty(
		type=properties.CGRUProperties,
		name='CGRU Settings',
		description='CGRU Settings'
	)


def unregister():
	del bpy.types.Scene.cgru
	bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
	register()

