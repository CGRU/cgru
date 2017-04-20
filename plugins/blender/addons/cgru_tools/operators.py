# -*- coding: utf-8 -*-

import imp
import time
import os
import sys
import re

import bpy

from . import utils

LAYER_TEXT_BLOCK = '''#
# layer '{0}'
#
import bpy
bpy.context.scene.render.use_sequencer = False
bpy.context.scene.render.use_compositing = False
layers = bpy.context.scene.render.layers
for layer in layers:
    layer.use = False
layers['{0}'].use = True
bpy.context.scene.render.filepath = bpy.context.scene.render.filepath \
    + '_' + "{0}" + '_'
'''

CMD_TEMPLATE = "blender -b \"{blend_scene}\" -y -E {render_engine} " \
        "{python_options}" "{output_options} -s @#@ " \
        "-e @#@ -j {frame_inc} -a"


class CGRU_Browse(bpy.types.Operator):
    bl_idname = "cgru.browse"
    bl_label = "Show job"

    def execute(self, context):
        import cgruconfig
        import webbrowser

        server_address = cgruconfig.VARS['af_servername']
        server_port = cgruconfig.VARS['af_serverport']
        webbrowser.open('http://%s:%s' % (server_address, server_port))

        return {"FINISHED"}


class CGRU_Submit(bpy.types.Operator):
    """Submit job to Afanasy Renderfarm"""

    bl_idname = "cgru.submit"
    bl_label = "Submit Job"

    def execute(self, context):
        sce = context.scene
        cgru_props = sce.cgru
        rd = context.scene.render
        images = None
        engine_string = sce.render.engine
        sceneModified = False  # if the opriginal scene modified checker

        # Import Afanasy module:
        import af

        # Calculate temporary scene path:
        scenefile = bpy.data.filepath
        if scenefile.endswith('.blend'):
            scenefile = scenefile[:-6]
        renderscenefile = "%s.%s.blend" % (
            scenefile,
            time.strftime('%Y%m%d%H%M%S'))

        # Make all Local and pack all textures and objects
        if cgru_props.packLinkedObjects:
            bpy.ops.object.make_local(type='ALL')
            sceneModified = True
        if cgru_props.relativePaths:
            bpy.ops.file.make_paths_relative()
            sceneModified = True
        if cgru_props.packTextures:
            bpy.ops.file.pack_all()
            sceneModified = True

        # Get job name:
        jobname = cgru_props.jobname
        # If job name is empty use scene file name:
        if not jobname:
            jobname = os.path.basename(scenefile)
            # Try to cut standart '.blend' extension:
            if jobname.endswith('.blend'):
                jobname = jobname[:-6]

        # Get frames settings:
        fstart = sce.frame_start
        fend = sce.frame_end
        finc = sce.frame_step
        fpertask = cgru_props.fpertask
        sequential = cgru_props.sequential

        # Check frames settings:
        if fpertask < 1:
            fpertask = 1
        if fend < fstart:
            fend = fstart

        # Create a job:
        job = af.Job(jobname)

        servicename = 'blender'
        renderlayer_names = []
        layers = bpy.context.scene.render.layers

        if cgru_props.splitRenderLayers and len(layers) > 1:
            for layer in layers:
                if layer.use:
                    renderlayer_names.append(layer.name)
        else:
            renderlayer_names.append('')

        for renderlayer_name in renderlayer_names:
            block = None
            images = None

            # Create block
            if cgru_props.splitRenderLayers and len(layers) > 1:
                txt_block = bpy.data.texts.new("layer_%s" % renderlayer_name)
                txt_block.write(LAYER_TEXT_BLOCK.format(renderlayer_name))
                block = af.Block("layer_%s" % renderlayer_name, servicename)
            else:
                block = af.Block(engine_string, servicename)

            # Check current render engine
            if engine_string == 'BLENDER_RENDER':
                block.setParser('blender_render')
            elif engine_string == 'CYCLES':
                block.setParser('blender_cycles')

            if cgru_props.filepath != '':
                pos = cgru_props.filepath.find('#')
                if pos != -1:
                    if cgru_props.filepath[pos-1] in '._- ':
                        images = "{0}{1}{2}".format(
                            cgru_props.filepath[:pos-1],
                            renderlayer_name,
                            cgru_props.filepath[pos-1:])
                    else:
                        images = "{0}{1}{2}".format(
                            cgru_props.filepath[:pos],
                            renderlayer_name,
                            cgru_props.filepath[pos:])
                else:
                    images = "{0}{1}".format(
                        cgru_props.filepath,
                        renderlayer_name)

                output_images = re.sub(r'(#+)', r'@\1@', images)
                if output_images.startswith('//'):
                    output_images = os.path.join(
                        os.path.dirname(renderscenefile),
                        output_images.replace('//', ''))

                if rd.file_extension not in output_images:
                    block.setFiles([output_images + rd.file_extension])
                else:
                    block.setFiles([output_images])

            if cgru_props.splitRenderLayers and len(layers) > 1:
                python_options = ' --python-text "layer_%s"' % renderlayer_name
            else:
                python_options = ''
            cmd = CMD_TEMPLATE.format(
                    blend_scene=renderscenefile,
                    render_engine=engine_string,
                    python_options=python_options,
                    output_options=' -o "%s" ' % images if images else '',
                    frame_inc=finc)

            block.setCommand(cmd)
            block.setNumeric(fstart, fend, fpertask, finc)
            block.setSequential(sequential)

            job.blocks.append(block)

            if cgru_props.make_movie:
                movie_block = af.Block(
                    cgru_props.mov_name + '-movie',
                    'movgen')
                movie_block.setDependMask(job.blocks[-1])
                movie_task = af.Task(cgru_props.mov_name)
                movie_block.tasks.append(movie_task)
                cmd = os.getenv('CGRU_LOCATION')
                cmd = os.path.join(cmd,
                                   'utilities',
                                   'moviemaker',
                                   'makemovie.py')
                cmd = 'python "%s"' % cmd
                cmd += ' --codec "%s"' % cgru_props.mov_codecs
                cmd += ' -r "%sx%s"' % (
                    cgru_props.mov_width,
                    cgru_props.mov_height)
                cmd += ' "%s"' % images.replace('@#', '#').replace('#@', '#')
                cmd += ' "%s"' % cgru_props.mov_name
                movie_task.setCommand(cmd)
                job.blocks.append(movie_block)

        # Set job running parameters:
        if cgru_props.maxruntasks > -1:
            job.setMaxRunningTasks(cgru_props.maxruntasks)
        if cgru_props.priority > -1:
            job.setPriority(cgru_props.priority)
        if cgru_props.dependmask != '':
            job.setDependMask(cgru_props.dependmask)
        if cgru_props.dependmaskglobal != '':
            job.setDependMaskGlobal(cgru_props.dependmaskglobal)
        if cgru_props.hostsmask != '':
            job.setHostsMask(cgru_props.hostsmask)
        if cgru_props.hostsmaskexclude != '':
            job.setHostsMaskExclude(cgru_props.hostsmaskexclude)
        if cgru_props.pause:
            job.offLine()
        if cgru_props.previewPendingApproval:
            job.setPPApproval()
        # Make server to delete temporary file after job deletion:
        job.setCmdPost('deletefiles "%s"' % os.path.abspath(renderscenefile))

        # Print job information:
        job.output(True)

        # Save Temporary file
        bpy.ops.wm.save_as_mainfile(filepath=renderscenefile, copy=True)

        # Clean up temp text blocks
        if cgru_props.splitRenderLayers and len(layers) > 1:
            for text in bpy.data.texts:
                if "layer_" in text:
                    bpy.data.texts.remove(text)

        #  Send job to server:
        result = job.send()
        if not result[0]:
            msg = (
                "An error occurred when submitting job to Afanasy."
                "Check console.")
            self.report({'ERROR'}, msg)
        else:
            msg = "Job id:%s successfully submit to Afanasy."
            self.report({'INFO'}, msg % result[1]['id'])

        # if opriginal scene is modified - we need to reload the scene file
        if sceneModified:
            bpy.ops.wm.open_mainfile(filepath=scenefile + ".blend")

        return {'FINISHED'}
