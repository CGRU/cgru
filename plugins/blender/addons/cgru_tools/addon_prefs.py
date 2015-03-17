# -*- coding: utf-8 -*-

import bpy


class CGRUAddonPreferences(bpy.types.AddonPreferences):
	bl_idname = __package__

	cgru_location = bpy.props.StringProperty(
		name="CGRU Root location",
		subtype="DIR_PATH")

	def draw(self, context):
		layout = self.layout
		row = layout.row()
		import os
		if 'CGRU_LOCATION' not in os.environ:
			row.label(text="Please, set CGRU install root location")
			row = layout.row()
			row.prop(self, "cgru_location")
		else:
			row.label(text="CGRU root location: %s" % os.getenv('CGRU_LOCATION'))

