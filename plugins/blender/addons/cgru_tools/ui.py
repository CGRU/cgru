# -*- coding: utf-8 -*-
import bpy
from . import operators
from . import utils


class RENDER_PT_Afanasy(bpy.types.Panel):
    bl_label = "Afanasy"
    bl_category = 'CGRU_props'
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "render"
    bl_options = {'DEFAULT_CLOSED'}

    def draw(self, context):
        layout = self.layout
        scene = context.scene
        cgru_props = scene.cgru

        col = layout.column()
        col.prop(cgru_props, 'jobname')
        col.prop(cgru_props, 'filepath')

        layout.separator()
        row = layout.row(align=False)
        col = row.column(align=True)
        col.prop(scene, 'frame_start')
        col.prop(scene, 'frame_end')
        col.prop(scene, 'frame_step')
        col = row.column(align=True)
        col.prop(cgru_props, 'fpertask')
        col.prop(cgru_props, 'sequential')
        col.prop(cgru_props, 'priority')
        col.prop(cgru_props, 'maxruntasks')

        layout.separator()
        row = layout.row()
        row.prop(cgru_props, 'adv_options')

        if cgru_props.adv_options:
            split = layout.split()
            col = split.column()
            col.prop(cgru_props, 'packLinkedObjects')
            col.prop(cgru_props, 'relativePaths')
            col.prop(cgru_props, 'packTextures')
            col = split.column()
            col.prop(cgru_props, 'pause')
            col.prop(cgru_props, 'splitRenderLayers')
            col.prop(cgru_props, 'previewPendingApproval')

            layout.separator()
            col = layout.column()
            col.prop(cgru_props, 'dependmask')
            col.prop(cgru_props, 'dependmaskglobal')
            col.prop(cgru_props, 'hostsmask')
            col.prop(cgru_props, 'hostsmaskexclude')

            layout.separator()
            col = layout.column()
            col.prop(cgru_props, 'make_movie')
            if cgru_props.make_movie:
                col.prop(cgru_props, 'mov_name')
                col.prop(cgru_props, 'mov_codecs')
                row = layout.row(align=True)
                row.prop(cgru_props, 'mov_width')
                row.prop(cgru_props, 'mov_height')

        layout.separator()
        row = layout.row(align=True)
        row.operator(operators.CGRU_Submit.bl_idname, icon='RENDER_STILL')
        row.operator(operators.CGRU_Browse.bl_idname)

        prefs = context.user_preferences.addons[__package__].preferences
        if prefs.cgru_version == utils.CGRU_NOT_FOUND:
            row.enabled = False
            layout.label(
                text="Please check CGRU location in the addon preferences.",
                icon='ERROR')
        else:
            row.enabled = True
