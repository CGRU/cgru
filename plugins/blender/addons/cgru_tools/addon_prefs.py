# -*- coding: utf-8 -*-

from bpy.types import AddonPreferences
from bpy.props import StringProperty
from . import utils


def update_location(self, context):
    self.cgru_version = utils.get_cgru_version(self.cgru_location)
    if self.cgru_version != utils.CGRU_NOT_FOUND:
        utils.add_cgru_module_to_syspath(self.cgru_location)


class CGRUAddonPreferences(AddonPreferences):
    bl_idname = __package__

    cgru_location = StringProperty(
        name="CGRU Root location",
        subtype="DIR_PATH",
        update=update_location)

    cgru_version = StringProperty(
        name="CGRU version",
        default="NOT FOUND")

    def draw(self, context):
        layout = self.layout
        col = layout.column()
        col.label(text="Please, set CGRU install root location")
        col.prop(self, "cgru_location")
        col.label(text="CGRU version: %s" % self.cgru_version)
