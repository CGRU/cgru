# -*- coding: utf-8 -*-
"""
  meArnoldRender

  Usage:
  You can add this code as shelf button :

from afanasy import meArnoldRender
reload( meArnoldRender )
meArnoldRender = meArnoldRender.meArnoldRender()

  Important!!!
  Do not use another object name.
  Only meArnoldRender.renderLayerSelected() will work with script job.

  Add %CGRU_LOCATION%\plugins\maya to %PYTHONPATH%

"""

import sys
import os
import string
import maya.OpenMaya as OpenMaya
import maya.cmds as cmds
import maya.mel as mel

from functools import partial

from maya_ui_proc import *
from afanasyRenderJob import *

self_prefix = 'meArnoldRender_'
meArnoldRenderVer = '0.0.1'
meArnoldRenderMainWnd = self_prefix + 'MainWnd'

ass_compression_list = ['Off', 'On']
ar_verbosity_list = [
    'none',
    'fatal',
    'error',
    'warning',
    'info',
    'progress',
    'debug',
    'details'
]


class meArnoldRender(object):
    """meArnoldRender
    """

    def __init__(self, selection=''):
        self.selection = selection
        self.winMain = ''

        self.os = sys.platform
        if self.os.startswith('linux'):
            self.os = 'linux'
        elif self.os == 'darwin':
            self.os = 'mac'
        elif self.os == 'win32':
            self.os = 'win'

        print('sys.platform = %s self.os = %s' % (sys.platform, self.os))

        if cmds.getAttr('defaultRenderGlobals.currentRenderer') != 'arnold':
            cmds.warning('Arnold is not current renderer!')
            return

        self.rootDir = cmds.workspace(q=True, rootDirectory=True)
        self.rootDir = self.rootDir[:-1]
        self.layer = cmds.editRenderLayerGlobals(
            query=True,
            currentRenderLayer=True
        )

        self.job = None

        self.job_param = {}
        self.ass_param = {}
        self.ar_param = {}
        self.img_param = {}
        self.afanasy_param = {}
        self.bkburn_param = {}

        self.assgenCommand = 'arnoldExportAss'
        self.def_assgenCommand = 'Render -r arnold'
        self.def_scene_name = ''  # maya scene name used for deferred .ass
                                  # generation

        self.save_frame_bgc = [0, 0, 0]
        self.def_frame_bgc = [0.75, 0.5, 0]

        self.initParameters()
        self.ui = self.setupUI()

    # def __del__( self ):
    #     print( ">> meArnoldRender: Class deleted" )

    def initParameters(self):
        """Missing DocString
        """

        #
        # Job parameters
        #
        self.job_param['job_dispatcher'] = \
            getDefaultStrValue(self_prefix, 'job_dispatcher', 'afanasy')

        self.job_param['job_name'] = getMayaSceneName()
        self.job_param['job_description'] = \
            getDefaultStrValue(self_prefix, 'job_description', '')

        self.job_param['job_animation'] = \
            getDefaultIntValue(self_prefix, 'job_animation', 1) is 1

        self.job_param['job_start'] = \
            getDefaultIntValue(self_prefix, 'job_start', 1)

        self.job_param['job_end'] = \
            getDefaultIntValue(self_prefix, 'job_end', 100)

        self.job_param['job_step'] = \
            getDefaultIntValue(self_prefix, 'job_step', 1)

        self.job_param['job_size'] = \
            getDefaultIntValue(self_prefix, 'job_size', 1)

        self.job_param['job_paused'] = \
            getDefaultIntValue(self_prefix, 'job_paused', 1) is 1

        self.job_param['job_priority'] = \
            getDefaultIntValue(self_prefix, 'job_priority', -1)

        self.job_param['job_cleanup_ass'] = \
            getDefaultIntValue(self_prefix, 'job_cleanup_ass', 0) is 1

        self.job_param['job_cleanup_script'] = \
            getDefaultIntValue(self_prefix, 'job_cleanup_script', 0) is 1

        #
        # .ass generation parameters
        #
        self.ass_param['ass_reuse'] = \
            getDefaultIntValue(self_prefix, 'ass_reuse', 0) is 1

        self.ass_param['ass_filename'] = 'data'

        self.ass_param['ass_padding'] = \
            getDefaultIntValue(self_prefix, 'ass_padding', 4)

        self.ass_param['ass_perframe'] = \
            getDefaultIntValue(self_prefix, 'ass_perframe', 1) is 1

        self.ass_param['ass_selection'] = \
            getDefaultIntValue(self_prefix, 'ass_selection', 0) is 1

        self.ass_param['ass_binary'] = \
            getDefaultIntValue(self_prefix, 'ass_binary', 0) is 1

        self.ass_param['ass_verbosity'] = \
            getDefaultStrValue(self_prefix, 'ass_verbosity', 'none')

        self.ass_param['ass_deferred'] = \
            getDefaultIntValue(self_prefix, 'ass_deferred', 0) is 1

        self.ass_param['ass_local_assgen'] = \
            getDefaultIntValue(self_prefix, 'ass_local_assgen', 1) is 1

        self.ass_param['ass_def_task_size'] = \
            getDefaultIntValue(self_prefix, 'ass_def_task_size', 4)

        self.ass_param['ass_export_all_layers'] = \
            getDefaultIntValue(self_prefix, 'ass_export_all_layers', 0) is 1

        self.ass_param['ass_compression'] = \
            getDefaultStrValue(self_prefix, 'ass_compression', 'Off')

        #
        # Arnold parameters
        #
        self.ar_param['ar_options'] = \
            getDefaultStrValue(self_prefix, 'ar_options', '')

        self.ar_param['ar_verbosity'] = \
            getDefaultStrValue(self_prefix, 'ar_verbosity', 'none')

        self.ar_param['ar_threads'] = \
            getDefaultIntValue(self_prefix, 'ar_threads', 0)

        self.ar_param['ar_distributed'] = \
            getDefaultIntValue(self_prefix, 'ar_distributed', 0) is 1

        self.ar_param['ar_nomaster'] = \
            getDefaultIntValue(self_prefix, 'ar_nomaster', 0) is 1

        self.ar_param['ar_hosts'] = \
            getDefaultStrValue(self_prefix, 'ar_hosts', '')  # @AF_HOSTS@

        self.ar_param['ar_port'] = \
            getDefaultIntValue(self_prefix, 'ar_port', 39010)

        self.ar_param['ar_hosts_min'] = \
            getDefaultIntValue(self_prefix, 'ar_hosts_min', 1)

        self.ar_param['ar_hosts_max'] = \
            getDefaultIntValue(self_prefix, 'ar_hosts_max', 4)

        self.ar_param['ar_threads_limit'] = \
            getDefaultIntValue(self_prefix, 'ar_threads_limit', 4)

        #
        # image parameters
        #
        self.img_param['img_filename'] = self.getImageFileNamePrefix()
        self.img_param['img_format'] = self.getImageFormat()

        #
        # Afanasy parameters
        #
        self.afanasy_param['af_capacity'] = \
            getDefaultIntValue(self_prefix, 'af_capacity', 1000)

        self.afanasy_param['af_deferred_capacity'] = \
            getDefaultIntValue( self_prefix, 'af_deferred_capacity', 1000)

        self.afanasy_param['af_use_var_capacity'] = \
            getDefaultIntValue(self_prefix, 'af_use_var_capacity', 0) is 1

        self.afanasy_param['af_cap_min'] = \
            getDefaultFloatValue(self_prefix, 'af_cap_min', 1.0)

        self.afanasy_param['af_cap_max'] = \
            getDefaultFloatValue(self_prefix, 'af_cap_max', 1.0)

        self.afanasy_param['af_max_running_tasks'] = \
            getDefaultIntValue(self_prefix, 'af_max_running_tasks', -1)

        self.afanasy_param['af_max_tasks_per_host'] = \
            getDefaultIntValue(self_prefix, 'af_max_tasks_per_host', -1)

        self.afanasy_param['af_service'] = \
            getDefaultStrValue(self_prefix, 'af_service', 'arnold')

        self.afanasy_param['af_deferred_service'] = \
            getDefaultStrValue(
                self_prefix,
                'af_deferred_service',
                'mayatoarnold'
            )

        self.afanasy_param['af_os'] = \
            getDefaultStrValue(self_prefix, 'af_os', '')  #linux mac windows

        self.afanasy_param['af_hostsmask'] = \
            getDefaultStrValue(self_prefix, 'af_hostsmask', '')

        self.afanasy_param['af_hostsexcl'] = \
            getDefaultStrValue(self_prefix, 'af_hostsexcl', '')

        self.afanasy_param['af_depmask'] = \
            getDefaultStrValue(self_prefix, 'af_depmask', '')

        self.afanasy_param['af_depglbl'] = \
            getDefaultStrValue(self_prefix, 'af_depglbl', '')

    def getImageFileNamePrefix(self):
        """Missing DocString
        """
        fileNamePrefix = cmds.getAttr('defaultRenderGlobals.imageFilePrefix')
        if fileNamePrefix == None or fileNamePrefix == '':
            fileNamePrefix = getMayaSceneName()
        return fileNamePrefix

    def getImageFormat(self):
        """Missing DocString
        """
        imageFormatStr = ''
        format_idx = cmds.getAttr('defaultRenderGlobals.imageFormat')
        if format_idx == 0:  # Gif
            imageFormatStr = 'iff'
        elif format_idx == 1:  # Softimage (pic)
            imageFormatStr = 'pic'
        elif format_idx == 2:  # Wavefront (rla)
            imageFormatStr = 'rla'
        elif format_idx == 3:  # Tiff
            imageFormatStr = 'tif'
        elif format_idx == 4:  # Tiff16
            imageFormatStr = 'iff'
        elif format_idx == 5:  # SGI (rgb)
            imageFormatStr = 'rgb'
        elif format_idx == 6:  # Alias (pix)
            imageFormatStr = 'pix'
        elif format_idx == 7:  # Maya IFF (iff)
            imageFormatStr = 'iff'
        elif format_idx == 8:  # JPEG (jpg)
            imageFormatStr = 'jpg'
        elif format_idx == 9:  # EPS (eps)
            imageFormatStr = 'eps'
        elif format_idx == 10:  # Maya 16 IFF (iff)
            imageFormatStr = 'iff'
        elif format_idx == 11:  # Cineon
            imageFormatStr = 'iff'
        elif format_idx == 12:  # Quantel PAL (yuv)
            imageFormatStr = 'yuv'
        elif format_idx == 13:  # SGI 16
            imageFormatStr = 'iff'
        elif format_idx == 19:  # Targa (tga)
            imageFormatStr = 'tga'
        elif format_idx == 20:  # Windows Bitmap (bmp)
            imageFormatStr = 'bmp'
        elif format_idx == 21:  # SGI Movie
            imageFormatStr = 'iff'
        elif format_idx == 22:  # Quicktime
            imageFormatStr = 'iff'
        elif format_idx == 23:  # AVI
            imageFormatStr = 'iff'
        elif format_idx == 30:  # MacPaint
            imageFormatStr = 'iff'
        elif format_idx == 31:  # PSD
            imageFormatStr = 'psd'
        elif format_idx == 32:  # PNG
            imageFormatStr = 'png'
        elif format_idx == 33:  # QuickDraw
            imageFormatStr = 'iff'
        elif format_idx == 34:  # QuickTime Image
            imageFormatStr = 'iff'
        elif format_idx == 35:  # DDS
            imageFormatStr = 'iff'
        elif format_idx == 36:  # PSD Layered
            imageFormatStr = 'psd'
        elif format_idx == 50:  # IMF plugin
            imageFormatStr = 'iff'
        elif format_idx == 51:  # Custom Image Format
            imageFormatStr = 'tif'
        elif format_idx == 60:  # Macromedia SWF (swf)
            imageFormatStr = 'iff'
        elif format_idx == 61:  # Adobe Illustrator (ai)
            imageFormatStr = 'iff'
        elif format_idx == 62:  # SVG (svg)
            imageFormatStr = 'iff'
        elif format_idx == 63:  # Swift3DImporter (swft)
            imageFormatStr = 'iff'
        return imageFormatStr

    def getDeferredCmd(self, layer=None):
        """Missing DocString
        """
        gen_cmd = self.def_assgenCommand
        gen_cmd += ' -proj "' + self.rootDir + '"'
        gen_cmd += ' -rt 1'  # Render type (
                             #     0 = render,
                             #     1 = export ass,
                             #     2 = export and kick
                             # )
        if layer is not None:
            gen_cmd += ' -rl ' + layer
        else:
            gen_cmd += ' -rl 1'
        gen_cmd += self.get_assgen_options(layer)
        return gen_cmd

    def getRenderCmd(self, layer=None):
        """Missing DocString

        :param layer: Missing documentation for layer argument
        :return:
        """
        ar_verbosity_level = \
            ar_verbosity_list.index(self.ar_param['ar_verbosity'])
        options = str(self.ar_param['ar_options']).strip()
        ar_threads = self.ar_param['ar_threads']

        if ar_verbosity_level < 2:
            ar_verbosity_level = 2

        cmd_buffer = ['kick']
        if ar_verbosity_level != 0:
            cmd_buffer.append('-v %s' % ar_verbosity_level)

        if ar_threads != 0:
            cmd_buffer.append('-t %s' % ar_threads)

        # cmd_buffer.append('-nstdin -dw -dp -nocrashpopup -i')
        # !!! -nocrashpopup flag doesn't support in kick linux version

        cmd_buffer.append('-nstdin -dw -dp -i')

        if len(options):
            cmd_buffer.append(options)

        return ' '.join(cmd_buffer)

    def get_ass_file_names(self, suffix=True, layer=None):
        """Missing DocString

        :param suffix:
        :param layer:
        :return:
        """
        ass_deferred = self.ass_param['ass_deferred']
        filename = self.ass_param['ass_filename']
        if layer is not None:
            if layer == 'defaultRenderLayer':
                layer = 'masterLayer'

            if not (len(getRenderLayersList(False)) == 1):
                filename += '/' + layer

        scenename = getMayaSceneName()

        if ass_deferred:
            scenename, ext = os.path.splitext(
                os.path.basename(self.def_scene_name)
            )

        filename += '/%s' % scenename
        filename = cmds.workspace(expandName=filename)

        if suffix:
            pad_str = getPadStr(
                self.ass_param['ass_padding'],
                self.ass_param['ass_perframe']
            )
            filename += '.@%s@.ass' % pad_str
            if self.ass_param['ass_compression'] != 'Off':
                filename += '.gz'

        return filename

    def get_image_names(self):
        """Missing DocString
        """
        pad_str = getPadStr(
            self.ass_param['ass_padding'],
            self.ass_param['ass_perframe']
        )

        images = cmds.renderSettings(
            fullPath=True,
            genericFrameImageName=('@%s@' % pad_str)
        )

        #imageFileName = ';'.join (images)
        imageFileName = str(images[0])
        return fromNativePath(imageFileName)

    def get_assgen_options(self, layer=None):
        """Missing DocString

        :param layer:
        :return:
        """
        animation = self.job_param['job_animation']
        start = self.job_param['job_start']
        stop = self.job_param['job_end']
        step = self.job_param['job_step']
        ass_reuse = self.ass_param['ass_reuse']
        ass_selection = self.ass_param['ass_selection']
        ass_filename = self.ass_param['ass_filename']
        ass_padding = self.ass_param['ass_padding']
        ass_perframe = self.ass_param['ass_perframe']
        ass_binary = self.ass_param['ass_binary']
        ass_deferred = self.ass_param['ass_deferred']

        assgen_cmd = ''
        filename = cmds.workspace(expandName=ass_filename)
        filename, ext = os.path.splitext(filename)

        if ext == '' or ext == '.':
            ext = '.ass'

        if ass_deferred:
            if ass_binary:
                assgen_cmd += ' -ai:bass 1'

            assgen_cmd += ' -ai:lfv 2'
            #assgen_cmd += ' -filename \"' + filename + '\"'
        else:
            if layer is not None:
                layer_in_filename = layer
                if layer == 'defaultRenderLayer':
                    layer_in_filename = 'masterLayer'

                filename += '_' + layer_in_filename

                # for deferred generation we do not add layer name to filename
                # this will be done by Maya

            filename = self.get_ass_file_names(False, layer)

            ass_compression_level = \
                ass_compression_list.index(self.ass_param['ass_compression'])

            if ass_compression_level > 0:
                assgen_cmd += ' -compressed'

            if not ass_binary:
                assgen_cmd += ' -asciiAss'

            if ass_selection:
                assgen_cmd += ' -selected'

            assgen_cmd += ' -expandProcedurals'
            assgen_cmd += ' -filename \"' + filename + '\"'

        return assgen_cmd

    def generate_ass(self, isSubmitingJob=False):
        """Missing DocString

        :param isSubmitingJob:
        :return:
        """
        skipExport = False
        exportAllRenderLayers = self.ass_param['ass_export_all_layers']

        animation = self.job_param['job_animation']
        start = self.job_param['job_start']
        stop = self.job_param['job_end']
        step = self.job_param['job_step']
        ass_reuse = self.ass_param['ass_reuse']
        ass_selection = self.ass_param['ass_selection']
        ass_filename = self.ass_param['ass_filename']
        ass_padding = self.ass_param['ass_padding']
        ass_perframe = self.ass_param['ass_perframe']
        ass_binary = self.ass_param['ass_binary']
        ass_deferred = self.ass_param['ass_deferred']

        filename = cmds.workspace(expandName=ass_filename)
        filename, ext = os.path.splitext(filename)

        if ext == '' or ext == '.':
            ext = '.ass'
        filename += ext

        dirname = os.path.dirname(filename)
        if not os.path.exists(dirname):
            print("path %s not exists" % dirname)
            os.mkdir(dirname)

        # TODO!!! check if files are exist and have to be overwritten
        if isSubmitingJob and ass_reuse:
            skipExport = True
            print("Skipping .ass files generation ...")

        if not skipExport:
            if not animation:
                start = stop = int(cmds.currentTime(q=True))
                step = 1
            #
            # save RenderGlobals
            #
            defGlobals = 'defaultRenderGlobals'
            saveGlobals = {}

            image_name = self.getImageFileNamePrefix()

            if ass_deferred:
                # generate unique maya scene name and save it
                # with current render and .ass generation settings
                print('Use deferred .ass generation')
                saveGlobals['imageFilePrefix'] = \
                    str(cmds.getAttr(defGlobals + '.imageFilePrefix'))

                cmds.setAttr(
                    defGlobals + '.imageFilePrefix',
                    image_name,
                    type='string'
                )

                scene_name = getMayaSceneName(withoutSubdir=False)  # get scene name without extension

                def_scene_name = scene_name + '_deferred'

                cmds.file(rename=def_scene_name)
                self.def_scene_name = cmds.file(save=True, de=True)  # save it with default extension
                cmds.file(rename=scene_name)  # rename scene back

                cmds.setAttr(
                    defGlobals + '.imageFilePrefix',
                    saveGlobals['imageFilePrefix'],
                    type='string'
                )
            else:
                renderLayers = []
                # save current layer
                current_layer = \
                    cmds.editRenderLayerGlobals(
                        q=True,
                        currentRenderLayer=True
                    )
                if exportAllRenderLayers:
                    renderLayers = getRenderLayersList(True)  # renderable only
                else:
                    # use only current layer
                    renderLayers.append(current_layer)

                for layer in renderLayers:
                    # if layer == 'masterLayer' :
                    #     layer = 'defaultRenderLayer' 
                    saveGlobals['renderableLayer'] = \
                        cmds.getAttr(layer + '.renderable')

                    cmds.setAttr(layer + '.renderable', True)
                    # print 'set current layer renderable (%s)' % layer

                    cmds.editRenderLayerGlobals(currentRenderLayer=layer)

                    assgen_cmd = \
                        self.assgenCommand + self.get_assgen_options(layer)

                    assgen_cmd += ' -startFrame %d' % start
                    assgen_cmd += ' -endFrame %d' % stop
                    assgen_cmd += ' -frameStep %d' % step

                    #cmds.setAttr( defGlobals + '.imageFilePrefix', image_name, type='string' ) # will use MayaSceneName if empty 

                    print( assgen_cmd)
                    mel.eval(assgen_cmd)

                    cmds.setAttr(
                        layer + '.renderable',
                        saveGlobals['renderableLayer']
                    )

                if exportAllRenderLayers:
                    # restore current layer
                    cmds.editRenderLayerGlobals(
                        currentRenderLayer=current_layer
                    )

    def submitJob(self, param=None):
        """Missing DocString

        :param param: Missing documentation of unused param argument
        :return:
        """

        job_dispatcher = self.job_param['job_dispatcher']
        job_description = self.job_param['job_description']
        job_name = str(self.job_param['job_name']).strip()
        if job_name == '':
            job_name = getMayaSceneName()

        ass_deferred = self.ass_param['ass_deferred']
        ass_local_assgen = self.ass_param['ass_local_assgen']
        ass_def_task_size = self.ass_param['ass_def_task_size']
        ass_reuse = self.ass_param['ass_reuse']
        exportAllRenderLayers = self.ass_param['ass_export_all_layers']

        if job_dispatcher == 'afanasy':
            self.job = AfanasyRenderJob(job_name, job_description)

            self.job.use_var_capacity = self.afanasy_param[
                'af_use_var_capacity']
            self.job.capacity_coeff_min = self.afanasy_param['af_cap_min']
            self.job.capacity_coeff_max = self.afanasy_param['af_cap_max']
            self.job.max_running_tasks = self.afanasy_param[
                'af_max_running_tasks']
            self.job.max_tasks_per_host = self.afanasy_param[
                'af_max_tasks_per_host']

            self.job.hostsmask = str(
                self.afanasy_param['af_hostsmask']).strip()
            self.job.hostsexcl = str(
                self.afanasy_param['af_hostsexcl']).strip()
            self.job.depmask = str(self.afanasy_param['af_depmask']).strip()
            self.job.depglbl = str(self.afanasy_param['af_depglbl']).strip()
            self.job.need_os = str(self.afanasy_param['af_os']).strip()

            service = str(self.afanasy_param['af_service']).strip()
            deferred_service = \
                str(self.afanasy_param['af_deferred_service']).strip()

            capacity = self.afanasy_param['af_capacity']
            deferred_capacity = self.afanasy_param['af_deferred_capacity']

        elif job_dispatcher == 'backburner':
            print('backburner not supported in this version')
            #self.job = ArnoldBackburnerJob ( job_name, job_description )
            return
        else:
            ass_deferred = False
            self.job = RenderJob(job_name, job_description)

        #self.job.work_dir  = self.rootDir
        self.job.padding = self.ass_param['ass_padding']
        self.job.priority = self.job_param['job_priority']
        self.job.paused = self.job_param['job_paused']
        self.job.task_size = self.job_param['job_size']
        self.job.animation = self.job_param['job_animation']
        self.job.start = self.job_param['job_start']
        self.job.stop = self.job_param['job_end']
        self.job.step = self.job_param['job_step']

        self.generate_ass(True)  # isSubmitingJob=True

        self.job.setup_range(int(cmds.currentTime(q=True)))
        self.job.setup()

        # save current layer
        current_layer = \
            cmds.editRenderLayerGlobals(
                q=True,
                currentRenderLayer=True
            )

        if job_dispatcher == 'afanasy':
            if ass_deferred and not ass_reuse:
                if exportAllRenderLayers:
                    gen_cmd = self.getDeferredCmd(None)
                else:
                    gen_cmd = self.getDeferredCmd(current_layer)
                gen_cmd += ' -s @#@'
                gen_cmd += ' -e @#@'
                gen_cmd += ' -b %s' % self.job.step
                print('gen_cmd = %s %s' % (gen_cmd, self.def_scene_name))

                self.job.gen_block = \
                    AfanasyRenderBlock(
                        'generate_ass',
                        deferred_service,
                        self.job,
                        ass_local_assgen
                    )
                self.job.gen_block.capacity = deferred_capacity
                self.job.gen_block.cmd = gen_cmd
                self.job.gen_block.input_files = '"%s"' % self.def_scene_name
                self.job.gen_block.task_size = min(ass_def_task_size,
                                                   self.job.num_tasks)
                self.job.gen_block.setup()

            renderLayers = []
            if exportAllRenderLayers:
                renderLayers = getRenderLayersList(True)  # renderable only
            else:
                # use only current layer
                renderLayers.append(current_layer)

            for layer in renderLayers:
                cmds.editRenderLayerGlobals(currentRenderLayer=layer)
                layer_name = layer
                if layer == 'defaultRenderLayer':
                    layer_name = 'masterLayer'

                frame_block = \
                    AfanasyRenderBlock(
                        'render_%s' % layer_name,
                        service,
                        self.job
                    )
                frame_block.capacity = capacity
                frame_block.input_files = '"%s"' % self.get_ass_file_names(
                    True, layer_name)
                frame_block.out_files = self.get_image_names()
                render_cmd = self.getRenderCmd(layer)

                if self.ar_param['ar_distributed']:
                    frame_block.distributed = True
                    frame_block.hosts_min = self.ar_param['ar_hosts_min']
                    frame_block.hosts_max = self.ar_param['ar_hosts_max']

                    if frame_block.hosts_max <= 0:
                        frame_block.hosts_max = 1

                    if frame_block.hosts_min > frame_block.hosts_max:
                        frame_block.hosts_min = 1

                    if self.ar_param['ar_nomaster']:
                        render_cmd += ' -nomaster '

                    hosts_str = str(self.ar_param['ar_hosts']).strip()

                    if hosts_str != '':
                        hosts = ' -hosts '
                        #hosts_list = hosts_str.split ( ' ' )
                        #for host in hosts_list :
                        #  hosts += ( '"%s:%d' % ( host, self.ar_param [ 'ar_port' ]) )
                        #  if self.ar_param [ 'ar_threads_limit' ] > 0 :
                        #    hosts += ( ' -threads %d' % self.ar_param [ 'ar_threads_limit' ] ) 
                        #  hosts += ( '" ' )
                        hosts = '%s%s' % (hosts, hosts_str)
                        render_cmd += '%s -- ' % hosts
                    else:
                        render_cmd += ' @AF_HOSTS@ '

                frame_block.cmd = render_cmd

                frame_block.setup()
                self.job.frames_blocks.append(frame_block)

            self.job.process()

        if exportAllRenderLayers:
            # restore current layer
            cmds.editRenderLayerGlobals(currentRenderLayer=current_layer)

    def assFileNameChanged(self, name, value):
        """Missing DocString

        :param name:
        :param value:
        """
        if name in ('ass_filename', 'ass_compression'):
            setDefaultStrValue(self_prefix, name, self.ass_param, value)
        else:  # ass_padding, ass_perframe
            setDefaultIntValue(self_prefix, name, self.ass_param, value)
        self.setResolvedPath()

    def setResolvedPath(self):
        """Missing DocString
        """
        filename = cmds.workspace(expandName=self.ass_param['ass_filename'])
        filename, ext = os.path.splitext(filename)

        if not (self.ass_param['ass_deferred'] and len(getRenderLayersList(False)) == 1):
            # do not add layer name for single render layer in scene
            # during deferred assgen
            filename += '_%s' % self.layer

        if self.ass_param['ass_padding'] > 0 and self.ass_param['ass_perframe']:
            filename += '.'
            pad_str = getPadStr(
                self.ass_param['ass_padding'],
                self.ass_param['ass_perframe']
            )
            filename += pad_str

        ext = '.ass'
        if self.ass_param['ass_compression'] != 'Off':
            ext += '.gz'

        filename += ext
        cmds.textFieldGrp(
            '%s|f0|t0|tc1|fr2|fc2|ass_resolved_path' % self.winMain,
            edit=True,
            text=filename
        )

    def enable_range(self, arg):
        """Missing DocString

        :param arg: Missing documentation of arg argument
        :return:
        """
        setDefaultIntValue(self_prefix, 'job_animation', self.job_param, arg)
        cmds.intFieldGrp(
             '%s|f0|t0|tc0|fr1|fc1|job_range' % self.winMain,
             edit=True,
             enable=arg
        )

    def enable_var_capacity(self, arg):
        """Missing DocString

        :param arg: Missing documentation about arg argument
        :return:
        """
        setDefaultIntValue(
            self_prefix,
            'af_use_var_capacity',
            self.afanasy_param,
            arg
        )

        cmds.floatFieldGrp(
            '%s|f0|t0|tc3|fr1|fc1|af_var_capacity' % self.winMain,
            edit=True,
            enable=arg
        )

    def enable_deferred(self, arg):
        """Missing DocString

        :param arg: Missing documentation about arg argument
        :return:
        """
        ass_def_frame = self.winMain + '|f0|t0|tc1|fr3'
        ass_def = ass_def_frame + '|fc3|'
        ass_compression = self.winMain + '|f0|t0|tc1|fr1|fc1|ass_compression'

        setDefaultIntValue(
            self_prefix,
            'ass_deferred',
            self.ass_param,
            arg
        )
        cmds.checkBoxGrp(
            ass_def + 'ass_local_assgen',
            edit=True,
            enable=arg
        )
        cmds.intFieldGrp(
            ass_def + 'ass_def_task_size',
            edit=True,
            enable=arg
        )
        cmds.optionMenuGrp(
            ass_compression,
            e=True,
            enable=not arg
        )

        bg_color = self.save_frame_bgc
        if arg:
            bg_color = self.def_frame_bgc

        cmds.frameLayout(
            ass_def_frame,
            edit=True,
            bgc=bg_color
        )  # , enableBackground=False

    def enable_distributed(self, arg):
        """Missing DocString

        :param arg: Missing documentation about arg argument
        :return:
        """
        ar_distr_frame = self.winMain + '|f0|t0|tc2|fr2'
        ar_distr = ar_distr_frame + '|fc2|'
        setDefaultIntValue(
            self_prefix,
            'ar_distributed',
            self.ar_param,
            arg
        )
        cmds.checkBoxGrp(
            ar_distr + 'ar_nomaster',
            edit=True,
            enable=arg
        )

        # cmds.intFieldGrp(
        #     ar_distr + 'ar_port',
        #     edit=True,
        #     enable=arg
        # )

        cmds.intFieldGrp(
            ar_distr + 'ar_hosts_min',
            edit=True,
            enable=arg
        )

        cmds.intFieldGrp(
            ar_distr + 'ar_hosts_max',
            edit=True,
            enable=arg
        )

        # cmds.intFieldGrp(
        #     ar_distr + 'ar_threads_limit',
        #     edit=True,
        #     enable=arg
        # )

        cmds.textFieldGrp(
            ar_distr + 'ar_hosts',
            edit=True,
            enable=arg
        )

        bg_color = self.save_frame_bgc
        if arg:
            bg_color = self.def_frame_bgc

        cmds.frameLayout(
            ar_distr_frame,
            edit=True,
            bgc=bg_color
        )  # , enableBackground=False

    def onRenderLayerSelected(self, arg):
        """Missing DocString

        :param arg: Missing documentation about arg argument
        :return:
        """
        #
        self.layer = arg
        if self.layer == 'masterLayer':
            arg = 'defaultRenderLayer'

        #print '* onRenderLayerSelected %s' % self.layer

        cmds.evalDeferred(
            'import maya.OpenMaya; '
            'maya.cmds.editRenderLayerGlobals('
            '   currentRenderLayer = "%s"'
            ')' % arg,
            lowestPriority=True
        )

    def renderLayerSelected(self):
        """Missing DocString
        """
        self.layer = cmds.editRenderLayerGlobals(
            query=True,
            currentRenderLayer=True
        )

        if self.layer == 'defaultRenderLayer':
            self.layer = 'masterLayer'

        print('* renderLayerSelected %s' % self.layer)

        cmds.optionMenuGrp(
            self.winMain + '|f0|c0|r0|' + 'layer_selector',
            e=True,
            value=self.layer
        )

        self.setResolvedPath()

    def renderLayerRenamed(self):
        """Missing DocString
        """
        self.layer = cmds.editRenderLayerGlobals(
            query=True,
            currentRenderLayer=True
        )

        #print('* renderLayerRenamed %s' % self.layer)

        # self.updateRenderLayerMenu()

        cmds.evalDeferred(
            partial(self.updateRenderLayerMenu),
            lowestPriority=True
        )

    def renderLayerChanged(self):
        """Missing DocString
        """
        self.layer = cmds.editRenderLayerGlobals(
            query=True,
            currentRenderLayer=True
        )
        #print('* renderLayerChanged %s' % self.layer)
        #self.updateRenderLayerMenu()
        cmds.evalDeferred(
            partial(self.updateRenderLayerMenu),
            lowestPriority=True
        )
        return True

    def updateRenderLayerMenu(self):
        """Missing DocString
        """
        list_items = cmds.optionMenuGrp(
            self.winMain + '|f0|c0|r0|' + 'layer_selector',
            q=True,
            itemListLong=True
        )

        if list_items is not None:
            # clear OptionMenu
            for item in list_items:
                cmds.deleteUI(item)

        renderLayers = getRenderLayersList(False)
        for layer in renderLayers:
            if layer == 'defaultRenderLayer':
                layer = 'masterLayer'
            cmds.menuItem(
                label=layer,
                parent=(self.winMain + '|f0|c0|r0|' + 'layer_selector|OptionMenu')
            )

        self.layer = cmds.editRenderLayerGlobals(
            query=True,
            currentRenderLayer=True
        )

        if self.layer == 'defaultRenderLayer':
            self.layer = 'masterLayer'

        cmds.optionMenuGrp(
            self.winMain + '|f0|c0|r0|' + 'layer_selector',
            e=True,
            value=self.layer
        )

        cmds.evalDeferred(
            partial(self.renderLayersSetup, renderLayers),
            lowestPriority=True
        )

    def renderLayersSetup(self, layers):
        """Missing DocString

        :param layers:
        :return:
        """
        # add script job for renaming render layer

        #print '* renderLayersSetup'
        selector = self.winMain + '|f0|c0|r0|layer_selector'
        firstRun = True
        for layer in layers:
            if layer != 'defaultRenderLayer':
                # cmds.scriptJob(
                #     nodeNameChanged=[layer, partial(self.renderLayerRenamed)],
                #     parent=top,
                #     replacePrevious=firstRun
                # )
                cmds.scriptJob(
                    nodeNameChanged=[
                        layer,
                        'import maya.OpenMaya; '
                        'maya.cmds.evalDeferred('
                        '    "meArnoldRender.renderLayerRenamed()",'
                        '    lowestPriority=True'
                        ')'
                    ],
                    parent=selector,
                    replacePrevious=firstRun
                )
                firstRun = False

    def setupUI(self):
        """Missing DocString
        """
        #
        # Main window setup
        #
        self.deleteUI(True)
        self.winMain = \
            cmds.window(
                meArnoldRenderMainWnd,
                title='meArnoldRender ver.%s (%s)' % (meArnoldRenderVer,
                                                      self.os),
                menuBar=True,
                retain=False,
                widthHeight=(420, 460)
            )

        self.mainMenu = cmds.menu(label='Commands', tearOff=False)
        cmds.menuItem(label='Render Globals ...', command=maya_render_globals)
        cmds.menuItem(label='Check Shaders ...', command=mrShaderManager)
        cmds.menuItem(label='Generate .ass', command=self.generate_ass)
        cmds.menuItem(label='Submit Job', command=self.submitJob)
        cmds.menuItem(divider=True)
        cmds.menuItem(label='Close', command=self.deleteUI)

        #
        # setup render layers script jobs
        #
        cmds.scriptJob(
            attributeChange=[
                'renderLayerManager.currentRenderLayer',
                'import maya.OpenMaya; '
                'maya.cmds.evalDeferred('
                '    "meArnoldRender.renderLayerSelected()",'
                '    lowestPriority=True'
                ')'
            ],
            parent=self.winMain
        )
        cmds.scriptJob(
            event=[
                'renderLayerChange',
                'import maya.OpenMaya; '
                'maya.cmds.evalDeferred('
                '    "meArnoldRender.renderLayerChanged()",'
                '    lowestPriority=True'
                ')'
            ],
            parent=self.winMain
        )

        cw1 = 120
        cw2 = 60
        cw3 = 20

        ar_hi = 8

        form = cmds.formLayout('f0', numberOfDivisions=100)
        proj = cmds.columnLayout(
            'c0',
            columnAttach=('left', 0),
            rowSpacing=2,
            adjustableColumn=True,
            height=50
        )
        cmds.textFieldGrp(
            cw=(1, 70),
            adj=2,
            label='Project Root ',
            text=self.rootDir,
            editable=False
        )  # , bgc=(0,0,0)

        cmds.rowLayout('r0', numberOfColumns=2)
        layer_selector = cmds.optionMenuGrp(
            'layer_selector',
            cw=((1, 70),),
            cal=(1, 'right'),
            label='Render Layer ',
            cc=partial(self.onRenderLayerSelected)
        )
        self.updateRenderLayerMenu()

        cmds.checkBoxGrp(
            'ass_export_all_layers',
            cw=((1, cw1), (2, cw1 * 2)),
            label='Export All Renderable ',
            value1=self.ass_param['ass_export_all_layers'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ass_export_all_layers',
                self.ass_param
            )
        )

        cmds.setParent('..')
        cmds.setParent('..')

        #
        # setup tabs
        #
        tab = cmds.tabLayout(
            't0',
            scrollable=True,
            childResizable=True
        )  # tabLayout -scr true -cr true  tabs; //

        #
        # Job tab
        #
        tab_job = cmds.columnLayout(
            'tc0',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.frameLayout(
            'fr1',
            label=' Parameters ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi
        )

        cmds.columnLayout(
            'fc1',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        job_dispatcher = cmds.optionMenuGrp(
            'job_dispatcher',
            cw=((1, cw1),),
            cal=(1, 'right'),
            label='Job Dispatcher ',
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'job_dispatcher',
                self.job_param
            )
        )

        for name in ('none', 'afanasy'):
            cmds.menuItem(label=name)  # 'backburner',

        cmds.optionMenuGrp(
            job_dispatcher,
            e=True,
            value=self.job_param['job_dispatcher']
        )

        cmds.text(label='')

        cmds.textFieldGrp(
            'job_name',
            cw=(1, cw1),
            adj=2,
            label='Job Name ',
            text=self.job_param['job_name'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'job_name',
                self.job_param
            )
        )

        cmds.textFieldGrp(
            'job_description',
            cw=(1, cw1),
            adj=2,
            label='Description ',
            text=self.job_param['job_description'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'job_description',
                self.job_param
            )
        )

        cmds.checkBoxGrp(
            'job_paused',
            cw=((1, cw1), (2, cw1 * 2)),
            label='Start Paused ',
            value1=self.job_param['job_paused'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'job_paused',
                self.job_param
            )
        )

        cmds.text(label='')
        cmds.checkBoxGrp(
            'job_animation',
            cw=((1, cw1), (2, cw1 * 2)),
            label='Animation ',
            value1=self.job_param['job_animation'],
            cc=partial(self.enable_range)
        )

        cmds.intFieldGrp(
            'job_range',
            cw=((1, cw1), (2, cw2), (3, cw2), (4, cw2)),
            nf=3,
            label='Start/Stop/By ',
            value1=self.job_param['job_start'],
            value2=self.job_param['job_end'],
            value3=self.job_param['job_step'],
            enable=self.job_param['job_animation'],
            cc=partial(
                setDefaultIntValue3,
                self_prefix,
                ('job_start', 'job_end', 'job_step'),
                self.job_param
            )
        )

        cmds.intFieldGrp(
            'job_size',
            cw=((1, cw1), (2, cw2)),
            label='Task Size ',
            ann='Should be smaller then number of frames to render',
            value1=self.job_param['job_size'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'job_size',
                self.job_param
            )
        )

        cmds.intFieldGrp(
            'job_priority',
            cw=((1, cw1), (2, cw2)),
            label='Priority ',
            value1=self.job_param['job_priority'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'job_priority',
                self.job_param
            )
        )

        cmds.setParent('..')
        cmds.setParent('..')

        cmds.frameLayout(
            'fr2',
            label=' Cleanup ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi,
            cll=True,
            cl=True
        )

        cmds.columnLayout(
            'fc2',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.checkBoxGrp(
            'job_cleanup_ass',
            cw=((1, cw1), (2, cw1 * 2)),
            label='',
            label1=' .ass files',
            value1=self.job_param['job_cleanup_ass'],
            enable=False,
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'job_cleanup_ass',
                self.job_param
            )
        )

        cmds.checkBoxGrp(
            'job_cleanup_script',
            cw=((1, cw1), (2, cw1 * 2)),
            label='',
            label1=' script file',
            value1=self.job_param['job_cleanup_script'],
            enable=False,
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'job_cleanup_script',
                self.job_param
            )
        )

        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #
        # .ass files generation tab
        #
        tab_assparam = cmds.columnLayout(
            'tc1',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.frameLayout(
            'fr3',
            label=' Deferred .ass generation ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi,
            cll=True,
            cl=True
        )

        cmds.columnLayout(
            'fc3',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.checkBoxGrp(
            'ass_deferred',
            cw=((1, cw1), (2, cw1 * 2)),
            label='Use deferred ',
            ann='Generate .ass files in background process',
            value1=self.ass_param['ass_deferred'],
            cc=partial(self.enable_deferred)
        )

        cmds.checkBoxGrp(
            'ass_local_assgen',
            cw=((1, cw1), (2, cw1 * 2)),
            label='',
            label1=" Only on localhost ",
            ann="Do not use remote hosts",
            value1=self.ass_param['ass_local_assgen'],
            enable=self.ass_param['ass_deferred'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ass_local_assgen',
                self.ass_param
            )
        )

        cmds.intFieldGrp(
            'ass_def_task_size',
            cw=((1, cw1), (2, cw2)),
            label='Task Size ',
            value1=self.ass_param['ass_def_task_size'],
            enable=self.ass_param['ass_deferred'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ass_def_task_size',
                self.ass_param
            )
        )

        #self.save_frame_bgc = cmds.frameLayout ( 'fr3', query = True, bgc = True )
        #self.def_frame_bgc = [ 0.75, 0.5, 0 ]
        bg_color = self.save_frame_bgc

        if self.ass_param['ass_deferred']:
            bg_color = self.def_frame_bgc

        cmds.frameLayout(
            self.winMain + '|f0|t0|tc1|fr3',
            edit=True,
            bgc=bg_color
        )  # , enableBackground=False

        cmds.setParent('..')
        cmds.setParent('..')

        cmds.frameLayout(
            'fr1',
            label=' Export Settings ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi
        )

        cmds.columnLayout(
            'fc1',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.checkBoxGrp(
            'ass_reuse',
            cw=((1, cw1), (2, cw1 * 2)),
            label='Use existing .ass files ',
            ann='Do not generate .ass files if they are exist',
            value1=self.ass_param['ass_reuse'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ass_reuse',
                self.ass_param
            )
        )

        cmds.text(label='')

        # ass_dir = cmds.textFieldButtonGrp(
        #     cw=(1, cw1),
        #     adj=2,
        #     label="Directory ",
        #     buttonLabel="...",
        #     text=self.ass_param['ass_dir']
        # )

        # cmds.textFieldButtonGrp(
        #     ass_dir,
        #     edit=True,
        #     bc=partial(browseDirectory, self.rootDir, ass_dir),
        #     cc=partial(self.setDefaultStrValue, 'ass_dir')
        # )


        ass_filename = cmds.textFieldButtonGrp(
            'ass_filename',
            cw=(1, cw1),
            enable=False,
            adj=2,
            label='File Name ',
            buttonLabel='...',
            text=self.ass_param['ass_filename'],
            cc=partial(self.assFileNameChanged, 'ass_filename')
        )

        cmds.textFieldButtonGrp(
            ass_filename,
            edit=True,
            bc=partial(
                browseFile,
                self.rootDir,
                ass_filename,
                'Arnold files (*.ass)'
            )
        )

        cmds.intFieldGrp(
            'ass_padding',
            cw=((1, cw1), (2, cw2)),
            label='Frame Padding ',
            value1=self.ass_param['ass_padding'],
            cc=partial(self.assFileNameChanged, 'ass_padding')
        )

        cmds.checkBoxGrp(
            'ass_perframe',
            cw=((1, cw1), (2, cw1 * 2)),
            label='',
            label1=' File Per Frame ',
            value1=self.ass_param['ass_perframe'],
            cc=partial(self.assFileNameChanged, 'ass_perframe')
        )

        cmds.checkBoxGrp(
            'ass_selection',
            cw=((1, cw1), (2, cw1 * 2)),
            label='',
            label1=' Export Only Selected Objects',
            value1=self.ass_param['ass_selection'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ass_selection',
                self.ass_param
            )
        )

        cmds.checkBoxGrp(
            'ass_binary',
            cw=((1, cw1), (2, cw1 * 2)),
            label='',
            label1=' Binary',
            value1=self.ass_param['ass_binary'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ass_binary',
                self.ass_param
            )
        )

        ass_compression = cmds.optionMenuGrp(
            'ass_compression',
            cw=((1, cw1),),
            cal=(1, 'right'),
            label='Compression ',
            cc=partial(self.assFileNameChanged, 'ass_compression')
        )

        for name in ass_compression_list:
            cmds.menuItem(label=name)

        cmds.optionMenuGrp(
            ass_compression,
            e=True,
            value=self.ass_param['ass_compression']
        )

        # ass_verbosity = none, fatal, error, warning, info, progress, and details
        # ass_verbosity = cmds.optionMenuGrp(
        #     'ass_verbosity',
        #     cw=((1, cw1),),
        #     cal=(1, 'right'),
        #     label="Verbosity ",
        #     cc=partial(self.setDefaultStrValue, 'ass_verbosity'))

        # for name in ('none', 'fatal', 'error', 'warning', 'info', 'progress', 'details'):
        #     cmds.menuItem(label=name)

        # cmds.optionMenuGrp(
        #     ass_verbosity,
        #     e=True,
        #     value=self.ass_param['ass_verbosity']
        # )

        cmds.setParent('..')
        cmds.setParent('..')
        cmds.frameLayout(
            'fr2',
            label=' Resolved Path ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi
        )

        cmds.columnLayout(
            'fc2',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.textFieldGrp(
            'ass_resolved_path',
            cw=(1, 0),
            adj=2,
            label='',
            text='',
            editable=False
        )

        self.setResolvedPath()

        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #
        # Renderer tab
        #
        tab_render = cmds.columnLayout(
            'tc2',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        fr2 = cmds.frameLayout(
            'fr2',
            label=' Distributed render ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi,
            cll=True,
            cl=True
        )

        cmds.columnLayout(
            'fc2',
            columnAttach=('left', 0), rowSpacing=0, adjustableColumn=True
        )

        cmds.checkBoxGrp(
            'ar_distributed',
            cw=((1, cw1), (2, cw1 * 2)),
            label='Use distributed ',
            enable=False,
            ann='Use slave hosts for rendering - is not available in Arnold.',
            value1=self.ar_param['ar_distributed'],
            cc=partial(self.enable_distributed)
        )

        cmds.checkBoxGrp(
            'ar_nomaster',
            cw=((1, cw1), (2, cw1 * 2)),
            label='',
            label1=" No Master ",
            ann="When rendering with multiple hosts, schedule all jobs on "
                "slaves only, if possible",
            value1=self.ar_param['ar_nomaster'],
            enable=self.ar_param['ar_distributed'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ar_nomaster',
                self.ar_param
            )
        )

        # cmds.intFieldGrp (
        #     'ar_port',
        #     cw=((1, cw1), (2, cw2)),
        #     label='Port ',
        #     value1=self.ar_param['ar_port'],
        #     enable=self.ar_param['ar_distributed'],
        #     cc=partial(
        #         setDefaultIntValue,
        #         self_prefix,
        #         'ar_port',
        #         self.ar_param
        #     )
        # )

        cmds.intFieldGrp(
            'ar_hosts_min',
            cw=((1, cw1), (2, cw2)),
            label='Min Hosts ',
            value1=self.ar_param['ar_hosts_min'],
            enable=self.ar_param['ar_distributed'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ar_hosts_min',
                self.ar_param
            )
        )

        cmds.intFieldGrp(
            'ar_hosts_max',
            cw=((1, cw1), (2, cw2)),
            label='Max Hosts ',
            value1=self.ar_param['ar_hosts_max'],
            enable=self.ar_param['ar_distributed'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ar_hosts_max',
                self.ar_param
            )
        )

        # cmds.intFieldGrp(
        #     'ar_threads_limit',
        #     cw=((1, cw1), (2, cw2)),
        #     label='Threads Limit ',
        #     ann='Max threads per host. All available threads will be used if 0',
        #     value1=self.ar_param['ar_threads_limit'],
        #     enable=self.ar_param['ar_distributed'],
        #     cc=partial(
        #         setDefaultIntValue,
        #         self_prefix,
        #         'ar_threads_limit',
        #         self.ar_param
        #     )
        # )

        cmds.textFieldGrp(
            'ar_hosts',
            cw=(1, cw1),
            adj=2,
            label='Remote Hosts ',
            ann='Remote hosts names (if empty, will be filled by Render '
                'Manager)',
            text=self.ar_param['ar_hosts'],
            enable=self.ar_param['ar_distributed'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'ar_hosts',
                self.ar_param
            )
        )

        bg_color = self.save_frame_bgc

        if self.ar_param['ar_distributed']:
            bg_color = self.def_frame_bgc

        cmds.frameLayout(
            fr2,
            edit=True,
            bgc=bg_color
        )  # , enableBackground=False

        cmds.setParent('..')
        cmds.setParent('..')

        cmds.frameLayout(
            'fr1',
            label=' Arnold options ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi
        )

        cmds.columnLayout(
            'fc1',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.textFieldGrp(
            'ar_options',
            cw=(1, cw1),
            adj=2,
            label='Additional Options ',
            text=self.ar_param['ar_options'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'ar_options',
                self.ar_param
            )
        )

        cmds.text(label='')

        ar_verbosity=cmds.optionMenuGrp(
            'ar_verbosity',
            cw=((1, cw1),),
            cal=(1, 'right'),
            label='Verbosity ',
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'ar_verbosity',
                self.ar_param
            )
        )

        for name in ar_verbosity_list:
            cmds.menuItem(label=name)

        cmds.optionMenuGrp(
            ar_verbosity,
            e=True,
            value=self.ar_param['ar_verbosity']
        )

        cmds.intFieldGrp(
            'ar_threads',
            cw=((1, cw1), (2, cw2)),
            label='Threads ',
            ann='The number of threads',
            value1=self.ar_param['ar_threads'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'ar_threads',
                self.ar_param
            )
        )

        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #
        # Afanasy tab
        #
        tab_afanasy = cmds.columnLayout(
            'tc3',
            columnAttach=('left', 0),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.frameLayout(
            'fr1',
            label=' Parameters ',
            borderVisible=True,
            borderStyle='etchedIn',
            marginHeight=ar_hi
        )

        cmds.columnLayout(
            'fc1',
            columnAttach=('left', 4),
            rowSpacing=0,
            adjustableColumn=True
        )

        cmds.intFieldGrp(
            'af_capacity',
            cw=((1, cw1), (2, cw2)),
            label='Task Capacity ',
            value1=self.afanasy_param['af_capacity'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'af_capacity',
                self.afanasy_param
            )
        )

        cmds.intFieldGrp(
            'af_deferred_capacity',
            cw=((1, cw1), (2, cw2)),
            label='Deferred Capacity ',
            value1=self.afanasy_param['af_deferred_capacity'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'af_deferred_capacity',
                self.afanasy_param
            )
        )

        cmds.checkBoxGrp(
            'af_use_var_capacity',
            cw=((1, cw1), (2, cw1 * 2)),
            label='Use Variable Capacity ',
            ann='Block can generate tasks with capacity*coefficient to fit '
                'free render capacity',
            value1=self.afanasy_param['af_use_var_capacity'],
            cc=partial(self.enable_var_capacity)
        )

        cmds.floatFieldGrp(
            'af_var_capacity',
            cw=((1, cw1), (2, cw2), (3, cw2), (4, cw2)),
            nf=2,
            pre=2,
            label='Min/Max coefficient ',
            value1=self.afanasy_param['af_cap_min'],
            value2=self.afanasy_param['af_cap_max'],
            enable=self.afanasy_param['af_use_var_capacity'],
            cc=partial(
                setDefaultFloatValue2,
                self_prefix,
                ('af_cap_min', 'af_cap_max'),
                self.afanasy_param
            )
        )

        cmds.intFieldGrp(
            'af_max_running_tasks',
            cw=((1, cw1), (2, cw2)),
            label='Max Running Tasks ',
            value1=self.afanasy_param['af_max_running_tasks'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'af_max_running_tasks',
                self.afanasy_param
            )
        )

        cmds.intFieldGrp(
            'af_max_tasks_per_host',
            cw=((1, cw1), (2, cw2)),
            label='Max Tasks Per Host ',
            value1=self.afanasy_param['af_max_tasks_per_host'],
            cc=partial(
                setDefaultIntValue,
                self_prefix,
                'af_max_tasks_per_host',
                self.afanasy_param
            )
        )

        cmds.textFieldGrp(
            'af_service',
            cw=(1, cw1),
            adj=2,
            label='Service ',
            text=self.afanasy_param['af_service'],
            enable=False,
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'af_service',
                self.afanasy_param
            )
        )

        cmds.textFieldGrp(
            'af_deferred_service',
            cw=(1, cw1),
            adj=2,
            label='Deferred Service ',
            text=self.afanasy_param['af_deferred_service'],
            enable=False,
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'af_deferred_service',
                self.afanasy_param
            )
        )

        cmds.textFieldGrp(
            'af_hostsmask',
            cw=(1, cw1),
            adj=2,
            label='Hosts Mask ',
            ann='Job run only on renders which host name matches this mask\n'
                'e.g.  .* or host.*',
            text=self.afanasy_param['af_hostsmask'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'af_hostsmask',
                self.afanasy_param
            )
        )

        cmds.textFieldGrp(
            'af_hostsexcl',
            cw=(1, cw1),
            adj=2,
            label='Exclude Hosts Mask ',
            ann='Job can not run on renders which host name matches this '
                'mask\n e.g.  host.* or host01|host02',
            text=self.afanasy_param['af_hostsexcl'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'af_hostsexcl',
                self.afanasy_param
            )
        )

        cmds.textFieldGrp(
            'af_depmask',
            cw=(1, cw1),
            adj=2,
            label='Depend Mask ',
            ann='Job will wait other user jobs which name matches this mask',
            text=self.afanasy_param['af_depmask'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'af_depmask',
                self.afanasy_param
            )
        )

        cmds.textFieldGrp(
            'af_depglbl',
            cw=(1, cw1),
            adj=2,
            label='Global Depend Mask ',
            ann='Job will wait other jobs from any user which name matches '
                'this mask',
            text=self.afanasy_param['af_depglbl'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'af_depglbl',
                self.afanasy_param
            )
        )

        cmds.textFieldGrp(
            'af_os',
            cw=(1, cw1),
            adj=2,
            label='Needed OS ',
            ann='windows linux mac',
            text=self.afanasy_param['af_os'],
            cc=partial(
                setDefaultStrValue,
                self_prefix,
                'af_os',
                self.afanasy_param
            )
        )

        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.tabLayout(
            tab,
            edit=True,
            tabLabel=(
                (tab_job, "Job"),
                (tab_assparam, ".ass"),
                (tab_render, "Renderer"),
                (tab_afanasy, "Afanasy")
            )
        )

        cmds.setParent(form)

        btn_sbm = cmds.button(
            label='Submit',
            command=self.submitJob,
            ann='Generate .ass files and submit to dispatcher'
        )

        btn_gen = cmds.button(
            label='Generate .ass',
            command=self.generate_ass,
            ann='Force .ass files generation'
        )
        btn_cls = cmds.button(label='Close', command=self.deleteUI)

        cmds.formLayout(
            form,
            edit=True,
            attachForm=(
                (proj, 'top', 0),
                (proj, 'left', 0),
                (proj, 'right', 0),
                (tab, 'left', 0),
                (tab, 'right', 0),
                (btn_cls, 'bottom', 0),
                (btn_gen, 'bottom', 0),
                (btn_sbm, 'bottom', 0),
                (btn_sbm, 'left', 0),
                (btn_cls, 'right', 0)
            ),
            attachControl=(
                (tab, 'top', 0, proj),
                (tab, 'bottom', 0, btn_sbm),
                (btn_gen, 'left', 0, btn_sbm),
                (btn_gen, 'right', 0, btn_cls)
            ),
            attachPosition=(
                (btn_sbm, 'right', 0, 33),
                (btn_gen, 'right', 0, 66),
                (btn_cls, 'left', 0, 66)
            )
        )

        cmds.showWindow(self.winMain)

        return form

    def deleteUI(self, param):
        """Missing DocString

        :param param:
        :return:
        """
        winMain = meArnoldRenderMainWnd

        if cmds.window(winMain, exists=True):
            cmds.deleteUI(winMain, window=True)

        if cmds.windowPref(winMain, exists=True):
            cmds.windowPref(winMain, remove=True)

print('meArnoldRender sourced ...')
