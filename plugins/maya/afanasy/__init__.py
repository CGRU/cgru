# -*- coding: utf-8 -*-
import copy
import os
import time
import logging
import functools

import pymel.core as pm


logger = logging.getLogger(__name__)
logger.setLevel(logging.WARNING)


class UI(object):
	"""The render UI
	"""

	def __init__(self):
		self.windows_name = 'cgru_afanasy_wnd'

		if pm.window(self.windows_name, exists=True):
			pm.deleteUI(self.windows_name)

		self.window = pm.window(self.windows_name, t='Afanasy')

		with pm.columnLayout(adj=True):
			labels_width = 90
			with pm.rowLayout(nc=4, adj=2, cw4=(labels_width, 40, 15, 15)):
				pm.text(l='Start Frame')
				start_time_int_field = pm.intField(
					'cgru_afanasy__start_frame',
					v=pm.optionVar.get('cgru_afanasy__start_frame_ov', 1)
				)
				pm.button(
					l='<',
					ann='Use minimum animation range',
					c=functools.partial(
						self.set_field_value,
						start_time_int_field,
						functools.partial(
							pm.playbackOptions,
							q=True,
							min=True
						)
					)
				)
				pm.button(
					l='<<',
					ann='Use minimum playback range',
					c=functools.partial(
						self.set_field_value,
						start_time_int_field,
						functools.partial(
							pm.playbackOptions,
							q=True,
							ast=True
						)
					)
				)

			with pm.rowLayout(nc=4, adj=2, cw4=(labels_width, 40, 15, 15)):
				pm.text(l='End Frame')
				end_time_int_field = pm.intField(
					'cgru_afanasy__end_frame',
					v=pm.optionVar.get('cgru_afanasy__end_frame_ov', 1)
				)
				pm.button(
					l='<',
					ann='Use maximum animation range',
					c=functools.partial(
						self.set_field_value,
						end_time_int_field,
						functools.partial(
							pm.playbackOptions,
							q=True,
							max=True
						)
					)
				)
				pm.button(
					l='<<',
					ann='Use maximum playback range',
					c=functools.partial(
						self.set_field_value,
						end_time_int_field,
						functools.partial(
							pm.playbackOptions,
							q=True,
							aet=True
						)
					)
				)

			with pm.rowLayout(nc=2, adj=2, cw2=(labels_width, 50)):
				pm.text(l='Frame Per Task')
				pm.intField(
					'cgru_afanasy__frames_per_task',
					v=pm.optionVar.get('cgru_afanasy__frames_per_task_ov', 1)
				)

			with pm.rowLayout(nc=2, adj=2, cw2=(labels_width, 50)):
				pm.text(l='By Frame')
				pm.intField(
					'cgru_afanasy__by_frame',
					v=pm.optionVar.get('cgru_afanasy__by_frame_ov', 1)
				)

			with pm.rowLayout(nc=2, adj=2, cw2=(labels_width, 50)):
				pm.text(l='Host Mask')
				pm.textField(
					'cgru_afanasy__hosts_mask',
					text='""'
				)

			with pm.rowLayout(nc=2, adj=2, cw2=(labels_width, 50)):
				pm.text(l='Host Exclude')
				pm.textField(
					'cgru_afanasy__hosts_exclude',
					text='""'
				)

			pm.checkBox('cgru_afanasy__paused', l='Start Paused', v=0)
			pm.checkBox(
				'cgru_afanasy__separate_layers',
				l='Submit Render Layers as Separate Tasks',
				v=1
			)

			pm.button(
				l='LAUNCH',
				c=self.launch
			)

			pm.checkBox(
				'cgru_afanasy__close',
				l='Close After',
				v=1
			)

		pm.showWindow(self.window)

	def set_field_value(self, control, value, *args, **kwargs):
		"""sets the given field value

		:param control: the UI control
		:param value: the value, can be a callable
		:return:
		"""
		try:
			v = value()
		except TypeError:
			v = value
		control.setValue(v)

	def launch(self, *args, **kwargs):
		"""launch renderer command
		"""
		# do nothing if there is no window (called externally)
		if not self.window:
			return

		# get values
		start_frame = pm.intField('cgru_afanasy__start_frame', q=1, v=1)
		end_frame = pm.intField('cgru_afanasy__end_frame', q=1, v=1)
		frames_per_task = \
			pm.intField('cgru_afanasy__frames_per_task', q=1, v=1)
		by_frame = pm.intField('cgru_afanasy__by_frame', q=1, v=1)
		hosts_mask = pm.textField('cgru_afanasy__hosts_mask', q=1, text=True)
		hosts_exclude = pm.textField('cgru_afanasy__hosts_exclude', q=1, text=True)
		separate_layers = \
			pm.checkBox('cgru_afanasy__separate_layers', q=1, v=1)
		pause = pm.checkBox('cgru_afanasy__paused', q=1, v=1)

		# check values
		if start_frame > end_frame:
			temp = end_frame
			end_frame = start_frame
			start_frame = temp

		frames_per_task = max(1, frames_per_task)
		by_frame = max(1, by_frame)

		# store field values
		pm.optionVar['cgru_afanasy__start_frame_ov'] = start_frame
		pm.optionVar['cgru_afanasy__end_frame_ov'] = end_frame
		pm.optionVar['cgru_afanasy__frames_per_task_ov'] = frames_per_task
		pm.optionVar['cgru_afanasy__by_frame_ov'] = by_frame
		pm.optionVar['cgru_afanasy__hosts_mask_ov'] = hosts_mask
		pm.optionVar['cgru_afanasy__hosts_exclude_ov'] = hosts_exclude
		pm.optionVar['cgru_afanasy__separate_layers'] = separate_layers

		# get paths
		scene_name = pm.sceneName()
		datetime = '%s%s' % (
			time.strftime('%y%m%d-%H%M%S-'),
			str(time.time() - int(time.time()))[2:5]
		)

		filename = '%s.%s.mb' % (scene_name, datetime)

		project_path = pm.workspace(q=1, rootDirectory=1)

		outputs = ','.join(
			pm.renderSettings(fullPath=1, firstImageName=1, lastImageName=1)
		)

		job_name = os.path.basename(scene_name)

		logger.debug('%ss %se %sr' % (start_frame, end_frame, by_frame))
		logger.debug('scene        = %s' % scene_name)
		logger.debug('file         = %s' % filename)
		logger.debug('job_name     = %s' % job_name)
		logger.debug('project_path = %s' % project_path)
		logger.debug('outputs      = %s' % outputs)

		if pm.checkBox('cgru_afanasy__close', q=1, v=1):
			pm.deleteUI(self.window)

		cmd_buffer = [
			'"%(filename)s"',
			'%(start)s',
			'%(end)s',
			'-by %(by_frame)s',
			'-hostsmask %(msk)s',
			'-hostsexcl %(exc)s',
			'-fpt %(fpt)s',
			'-name "%(name)s"',
			'-proj "%(proj)s"',
			'-images "%(images)s"',
			'-deletescene'
		]

		kwargs = {
			'filename': filename,
			'start': start_frame,
			'end': end_frame,
			'by_frame': by_frame,
			'msk': hosts_mask,
			'exc': hosts_exclude,
			'fpt': frames_per_task,
			'name': job_name,
			'proj': project_path,
			'images': outputs
		}

		drg = pm.PyNode('defaultRenderGlobals')
		if drg.getAttr('currentRenderer') == 'mentalRay':
			cmd_buffer.append('-type maya_mental')

		if pause:
			cmd_buffer.append('-pause')

		# save file
		pm.saveAs(
			filename,
			force=1,
			type='mayaBinary'
		)

		# rename back to original name
		pm.renameFile(scene_name)

		cmds = []

		# submit renders
		if separate_layers:
			# render each layer separately
			rlm = pm.PyNode('renderLayerManager')
			layers = [layer for layer in rlm.connections()
					  if layer.renderable.get()]

			for layer in layers:
				layer_name = layer.name()
				kwargs['name'] = '%s:%s' % (job_name, layer_name)

				tmp_cmd_buffer = copy.copy(cmd_buffer)
				tmp_cmd_buffer.append(
					'-take %s' % layer.name()
				)

				# create one big command
				afjob_cmd = ' '.join([
					os.environ['CGRU_PYTHONEXE'],
					'"%s/python/afjob.py"' % os.environ['AF_ROOT'],
					'%s' % ' '.join(tmp_cmd_buffer) % kwargs
				])
				cmds.append(afjob_cmd)

		else:
			# create one big command
			afjob_cmd = ' '.join([
				os.environ['CGRU_PYTHONEXE'],
				'%s/python/afjob.py' % os.environ['AF_ROOT'],
				'%s' % ' '.join(cmd_buffer) % kwargs
			])
			cmds.append(afjob_cmd)

		# call each command separately
		for cmd in cmds:
			print(cmds)

			os.system(cmd)
