import re, os, shutil, time, sys

import webbrowser

import af

detailed_log = 0

#Get handles to the xsi application
#Application = win32com.client.Dispatch('XSI.Application')


def Log(logstring = ''):
	if (detailed_log == 1):
		Application.LogMessage('Debug__ "%s"' % logstring)
	

def CloseButton_OnClicked():
	PPG.Close()


def HelpButton_OnClicked():
	path = 'http://cgru.info/#xsi'
	Application.LogMessage('Opening "%s"' % path)
	webbrowser.open(path)


def SubmitButton_OnClicked():
	opSet = Application.ActiveSceneRoot.Properties('afSubmitProperties')
	if(opSet == None):
		Application.LogMessage('Error: Can\'t find options.')
	PPG.Close()

	# Save scene:
	#Application.SaveScene()
	scene = Application.ActiveProject.ActiveScene
	scenefile = scene.Filename.Value
	if not os.path.isfile(scenefile):
		Application.LogMessage('Error: you need to save first.', 2)
		return
	

	range_frompass		= opSet.Parameters('afRange_frompass'  ).Value
	range_forcepass		= opSet.Parameters('afRange_forcepass' ).Value
	frame_start			= opSet.Parameters('afFrame_start'     ).Value
	frame_end         	= opSet.Parameters('afFrame_end'       ).Value
	frame_by          	= opSet.Parameters('afFrame_by'        ).Value
	frame_fpt         	= opSet.Parameters('afFrame_fpt'       ).Value
	passesOption      	= opSet.Parameters('afRenderPass'      ).Value
	jobname           	= opSet.Parameters('afJobName'         ).Value
	priority          	= opSet.Parameters('afPriority'        ).Value
	capacity          	= opSet.Parameters('afCapacity'        ).Value
	simulate          	= opSet.Parameters('afSimulate'        ).Value
	paused            	= opSet.Parameters('afStartPaused'     ).Value
	maxhosts          	= opSet.Parameters('afMaxHosts'        ).Value
	maxruntime        	= opSet.Parameters('afMaxRunTime'      ).Value
	hostsmask         	= opSet.Parameters('afHostsMask'       ).Value
	hostsmaskexclude  	= opSet.Parameters('afHostsMaskExclude').Value
	dependmask        	= opSet.Parameters('afDependMask'      ).Value
	dependmaskglobal  	= opSet.Parameters('afDependMaskGlobal').Value
	varirender        	= opSet.Parameters('afVariRender'      ).Value
	varirender_attr   	= opSet.Parameters('afVariRenderAttr'  ).Value
	varirender_start  	= opSet.Parameters('afVariRenderStart' ).Value
	varirender_step   	= opSet.Parameters('afVariRenderStep'  ).Value
	varirender_count  	= opSet.Parameters('afVariRenderCount' ).Value
	ArnoldWatermarked 	= opSet.Parameters('afArnoldWater'     ).Value
	TempScenePath     	= opSet.Parameters('afTempScenePath'   ).Value
	ArnoldAutoThread  	= opSet.Parameters('afArnoldAutoThread').Value
	UseTemp				= opSet.Parameters('afUseTemp'   	   ).Value
	SkipFrame  			= opSet.Parameters('afSkipFrame'       ).Value
	ArnoldThreadsBool	= opSet.Parameters('afArnoldThreadsBool'   ).Value
	ArnoldThreadsCount  = opSet.Parameters('afArnoldThreadsCount'  ).Value
	Bucket 				= opSet.Parameters('afBucket'          ).Value
	Stillimage          = opSet.Parameters('afStillimage'      ).Value
	Progressive  		= opSet.Parameters('afProgressive'     ).Value
	RayReserved			= opSet.Parameters('afRayReserved'	   ).Value

	if frame_end < frame_start: frame_end = frame_start
	if frame_by  < 1: frame_by  = 1
	if frame_fpt < 1: frame_fpt = 1

	passes = []
	if passesOption == '_all_':
		for cpass in scene.Passes:
			passes.append( cpass.Name)
	elif passesOption == '_selected_':
		selection = []
		for selected in Application.Selection:
			selection.append( selected.Name)
		for cpass in scene.Passes:
			if cpass.Name in selection:
				passes.append( cpass.Name)
	elif passesOption == '_current_':
		passes.append( scene.ActivePass.Name)
	else:
		passes.append( passesOption)

	padding = Application.GetValue('Passes.RenderOptions.FramePadding')
	sceneRenderer = Application.GetValue('Passes.RenderOptions.Renderer')
	
	Application.logmessage(sceneRenderer)

	"""
	# CHECK SETTINGS
	# Frame Set
	range_setting = Application.GetValue('Passes.RenderOptions.FrameRangeSource')
	Application.logmessage('BLAAAAAA: %s' % range_setting)
	if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 1:
		Application.LogMessage('Sorry _Frame Set_ is not yet supported....aborting..., Pass:%s ' % cpass, 2)
		return
	# Timeline
	if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 2:
		Application.LogMessage('Sorry _Timeline_ is not supported....aborting..., Pass:%s ' % cpass, 2)
		return
	"""

	#just run through to set new settings....
	for cpass in passes:


		# Get framebuffers:
		for ps in scene.Passes:
			if ps.Name != cpass: continue
			
			curRenderer = Application.GetValue('Passes.%s.Renderer' % cpass)
			if (curRenderer == ''):
				curRenderer = sceneRenderer
								
			Application.logmessage('Scene Renderer: %s' % curRenderer)


			# SkipFrames
			if (SkipFrame == True):
				Application.SetValue('Passes.%s.FrameSkipRendered' % cpass, True, '')
				Application.logmessage('skip frames: ON')
			else:
				Application.SetValue('Passes.%s.FrameSkipRendered' % cpass, False, '')
				Application.logmessage('skip frames: OFF')


			# MENTAL RAY SPECIFIC
			if (curRenderer == 'Mental Ray'):
				nudel = Application.SetValue('Passes.%s.mentalray.TileSize' % cpass, Bucket, '')


			# ARNOLD SPECIFIC
			if (curRenderer == 'Arnold Render'):
				Application.logmessage('found Arnold pass, setting new values.....')
								
				# reset license values
				Log('-- just to be sure, reset of license checkboxes --')
				nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.skip_license_check' % cpass, True, '')
				nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.skip_license_check' % cpass, False, '')
				nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.abort_on_license_fail' % cpass, False, '')
				
				# Arnold watermarked?
				if ArnoldWatermarked:
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.skip_license_check' % cpass, True, '')
					Application.logmessage('set to watermarked.....')
				else:
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.abort_on_license_fail' % cpass, True, '')
			
				# force autothread?
				if ArnoldAutoThread:
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.autodetect_threads' % cpass, 1, '')
				else:
					if ArnoldThreadsBool:
						Application.logmessage('limiting threads......')
						nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.autodetect_threads' % cpass, 0, '')
						nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.threads' % cpass, ArnoldThreadsCount, '')

				# bucketsize
				Application.logmessage('forcing bucketsize....')
				nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.bucket_size' % cpass, Bucket, '')
				
				if Stillimage:
					Application.logmessage('tiled EXRs, no autocrop, top tiles')
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.bucket_scanning' % cpass, "top", '')
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.output_exr_tiled' % cpass, 1, '')
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.output_exr_autocrop' % cpass, False, '')
				else:
					Application.logmessage('scanline EXRs, autocrop, spiral tiles')
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.bucket_scanning' % cpass, "spiral", '')
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.output_exr_tiled' % cpass, 0, '')
					nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.output_exr_autocrop' % cpass, True, '')
				
				# force logging
				nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.enable_log_console' % cpass, 1, '')
				nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.enable_log_file' % cpass, 1, '')
				nudel = Application.SetValue('Passes.%s.Arnold_Render_Options.log_level' % cpass, 2, '')
			
			# REDSHIFT SPECIFIC			
			if (curRenderer == 'Redshift'):
				Application.logmessage('found Redshift pass, setting new values.....')


				if Progressive:
					nudel = Application.SetValue('Passes.%s.Redshift_Options.ProgressiveRenderingEnabled' % cpass, 1, '')
				else:
					nudel = Application.SetValue('Passes.%s.Redshift_Options.ProgressiveRenderingEnabled' % cpass, 0, '')
					
				if Stillimage:
					Application.logmessage('tiled EXRs, no autocrop, top tiles')
					nudel = Application.SetValue('Passes.%s.Redshift_Options.ExrIsTiled' % cpass, 1, '')
					nudel = Application.SetValue('Passes.%s.Redshift_Options.Autocrop' % cpass, False, '')
					nudel = Application.SetValue('Passes.%s.Redshift_Options.BucketOrder' % cpass, 2, '')
				else:
					Application.logmessage('scanline EXRs, autocrop, spiral tiles')
					nudel = Application.SetValue('Passes.%s.Redshift_Options.ExrIsTiled' % cpass, 0, '')
					nudel = Application.SetValue('Passes.%s.Redshift_Options.Autocrop' % cpass, True, '')
					nudel = Application.SetValue('Passes.%s.Redshift_Options.BucketOrder' % cpass, 1, '')

				# force logging
				nudel = Application.SetValue('Passes.%s.Redshift_Options.LogLevel' % cpass, 2, '')

				# bucketsize
				Application.logmessage('forcing bucketsize....')
				nudel = Application.SetValue('Passes.%s.Redshift_Options.BucketSize' % cpass, Bucket, '')

				# rayreserved
				Application.logmessage('forcing rayreservedmemory....')
				nudel = Application.SetValue('Passes.%s.Redshift_Options.NumGPUMBToReserveForRays' % cpass, RayReserved, '')

	# save the changes
	Application.SaveScene()

	framenumbers = []
	
	
	
	# run through and output the jobs ---------------------------------------------------------
	for cpass in passes:
		images = []
		images_array = []
	
		# Get frame range:
		cp_frame_start = frame_start
		cp_frame_end = frame_end
		cp_frame_by = frame_by
		
		if not range_forcepass:
			# Frame Range
			if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 0:
				cp_frame_start = Application.GetValue('Passes.%s.FrameStart' % cpass)
				cp_frame_end = Application.GetValue('Passes.%s.FrameEnd' % cpass)
				cp_frame_by = Application.GetValue('Passes.%s.FrameStep' % cpass)
				Application.LogMessage('found _Frame Range_ setting...')
			# Frame Set
			if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 1:
				Application.LogMessage('Sorry _Frame Set_ is not yet supported....aborting..., Pass:%s ' % cpass, 2)
				return
			# Timeline
			if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 2:
				#cp_frame_start = Application.GetValue('PlayControl.In')
				#cp_frame_end = Application.GetValue('PlayControl.Out')
				#cp_frame_by = 1
				#Application.LogMessage('found _Timeline_ setting...')
				Application.LogMessage('Sorry _Timeline_ is not supported....aborting..., Pass:%s ' % cpass, 2)
				return
			# Scene Render Options
			if Application.GetValue('Passes.%s.FrameRangeSource' % cpass) == 3:
				cp_frame_start = Application.GetValue('Passes.RenderOptions.FrameStart')
				cp_frame_end = Application.GetValue('Passes.RenderOptions.FrameEnd')
				cp_frame_by = Application.GetValue('Passes.RenderOptions.FrameStep')
				Application.LogMessage('found _Scene Render Options_ setting...')
				
	
		
		
		# Get framebuffers:
		for ps in scene.Passes:
			if ps.Name != cpass: continue
			
			curRenderer = Application.GetValue('Passes.%s.Renderer' % cpass)
			if (curRenderer == ''):
				curRenderer = sceneRenderer
								
			Log(curRenderer)
			for fb in ps.Framebuffers:
				if fb.Enabled.Value:
					format = fb.Format.Value
					filename = fb.ResolvedFilename.Value
					pattern = r'\d+.' + format + '$'
					match = re.search( pattern, filename)
									
					if match is not None:
						part = match.group(0)               
						match = re.search(r'\d+', part)

						if match is not None:
							num = match.group(0)

							pad = 'PADolgaPAD'
							framenumbers.append( num)

							newpart = part.replace( num, pad)
							filename = filename.replace( part, newpart)
							Log('renderfilename Path: "%s"' % filename)

							#images.append(filename)
							images_array.append(filename)
							
					else:
						Application.LogMessage('Can`t solve "%s". Exiting.....' % filename, 2)
						return
						
		
					
		
		# Copy scene to temporary file:
		curjobname = jobname
		if len(passes) > 1: curjobname += '-%s' % cpass
		ftime = time.time()
		if UseTemp:
			tmpscene = TempScenePath + curjobname + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + '.scn'
		else:
			tmpscene = scenefile + '.' + curjobname + time.strftime('.%m%d-%H%M%S-') + str(ftime - int(ftime))[2:5] + '.scn'
		try:
			# shutil.copyfile( scenefile, tmpscene)

			if sys.platform == 'win32':
				os.popen('cmd /C copy "%s" "%s" /Y' % (scenefile, tmpscene),'r')
				Application.LogMessage('Windows platform detected.. using copy "%s" "%s" /Y' % (scenefile, tmpscene))
			else:
				shutil.copy(scenefile, tmpscene)


		except:
			Application.LogMessage('Unable to copy temporary scene:', 2)
			Application.LogMessage( tmpscene)
			Application.LogMessage( str(sys.exc_info()[1]))
			return
		if not os.path.isfile( tmpscene):
			Application.LogMessage('Error: Can\'t save temporary scene.', 2)
			return


				
		# Construct job:
		# -----------------------------------------------------------------------------
		
		if (curRenderer == 'Arnold Render'):
			if ArnoldWatermarked:
				Application.LogMessage('Sending Arnold pass "%s" watermarked, range: %d-%d,%d' % (cpass, cp_frame_start, cp_frame_end, cp_frame_by))
			else:
				Application.LogMessage('Sending Arnold pass "%s" licensed, range: %d-%d,%d' % (cpass, cp_frame_start, cp_frame_end, cp_frame_by))
		
		if (curRenderer == 'mental ray'):
			Application.LogMessage('Sending MentalRay pass "%s", range: %d-%d,%d' % (cpass, cp_frame_start, cp_frame_end, cp_frame_by))
		
		if (curRenderer == 'Redshift'):
			Application.LogMessage('Sending Redshift pass "%s", range: %d-%d,%d' % (cpass, cp_frame_start, cp_frame_end, cp_frame_by))
			
		#xsibatch -script "%XSI_CGRU_PATH%\afrender.py" -lang Python -main afRenderCurPass -args
		#-scenePath "%CD%\project\Scenes\scene.scn" -startFrame 1 -endFrame 2 -step 1 -simulate 0 -setAttr torus.polymsh.geom.enduangle -setValue 120

		blocknames = []
		blockcmds = []
		blockimages = []
		#blockframenumbers = []
		images_str = ''

		cmd = os.environ['XSI_CGRU_PATH']
		cmd = os.path.join( cmd, 'afrender.py')
		cmd = 'xsibatch -script %s' % cmd
		cmd += ' -lang Python -main afRender -args'
		cmd += ' -scene "%s"' % tmpscene
		cmd += ' -start @#@ -end @#@ -step ' + str(cp_frame_by)
		cmd += ' -simulate'
		if simulate:   cmd += ' 1'
		else:          cmd += ' 0'
		cmd += ' -renderPass ' + cpass
		if varirender:
			cmd += ' -attr ' + varirender_attr + ' -value '
			value = varirender_start
			for i in range( 0, varirender_count):
				blockcmds.append( cmd + str(value))
				blocknames.append( 'var_value [%d]' % value)
				images_str = ''
				for img in images:
					img_dir = os.path.dirname( img)
					img_name = os.path.basename( img)
					img_dir = os.path.join( img_dir, str(value))
					img = os.path.join( img_dir, img_name)
					if images_str != '': images_str += ';'
					images_str += img
					images_array.append(img)

				blockimages.append(images_str)
				value += varirender_step
		else:
			# label the Blocks
			if (curRenderer == 'Arnold Render'):
				if ArnoldWatermarked:
					blockname = 'Arnold watermarked'
				else:
					blockname = 'Arnold LICENSED'
			
			if (curRenderer == 'mental ray'):
				blockname = 'Mental Ray'
			
			if (curRenderer == 'Redshift'):
				blockname = 'Redshift' 
			
			#Fallback, every other renderer
			if (curRenderer != 'Redshift') and (curRenderer != 'mental ray') and (curRenderer != 'Arnold Render'):
				blockname = 'Mental Ray'
				Application.LogMessage('Warning: selected Renderer not directly supported, Afanasy will treat it like a MentalRay job, but will try to render it with the selected renderer')
			
			
			
		#Log(images_array)
			
				
		job=af.Job( curjobname + ' -- ' + cpass)
		job.setCmdPost( str('deletefiles "%s"' % os.path.abspath(tmpscene)))
		if priority  != -1: job.setPriority( priority)
		if maxhosts  != -1: job.setMaxRunningTasks( maxhosts)
		if hostsmask         != None and hostsmask         != '': job.setHostsMask( hostsmask)
		if hostsmaskexclude  != None and hostsmaskexclude  != '': job.setHostsMaskExclude( hostsmaskexclude)
		if dependmask        != None and dependmask        != '': job.setDependMask( dependmask)
		if dependmaskglobal  != None and dependmaskglobal  != '': job.setDependMaskGlobal( dependmaskglobal)
		if paused: job.offLine()

		if len( blocknames) == 0:
			Log("inside the blocknames is zero ifblock")
			blocknames.append( blockname)
			blockcmds.append( cmd)
			#blockimages.append( images_str)
			blockimages.append(images_array)



		# SET IMAGE PREVIEWS --------------------------------------------------
		i = 0
		fb_index = 0

		for blockname in blocknames:
			# counter for the image array
			currentFrame = cp_frame_start

			for framebufferPaths in blockimages[i]:
				Log('FOR BLOCK: "%d"' % fb_index)
				Log('array size: "%d"' % len(images_array))	

				#Every Frame gets the correct padded first frame of the sequence for viewers like djv
				if padding > 1: 
					p = 0				
					Log('original framebufferPaths: "%s"' % framebufferPaths)
					Log('padding: "%d"' % padding)
					if len(str(currentFrame)) < padding:
						pad = str(currentFrame)
						for p in range(0, ( padding - len(str(currentFrame)))):
							pad = '0' + pad
					else:
						pad = str(currentFrame)


				framebufferPaths = framebufferPaths.replace('PADolgaPAD', pad)
				blockimages[i][fb_index] = str(framebufferPaths)

				Log('blockimage after : "%s"' % blockimages[i][fb_index])
				Log('framebufferPaths after : "%s"' % framebufferPaths)
				Log('pad after replace: "%s"' % pad)
		
				fb_index = fb_index + 1
				#m = m + cp_frame_by	
				
			i = i + 1
			


		# Set SERVICES --------------------------------------------------
		i = 0
		for blockname in blocknames:
			Log(' blockname: "%s"' % blockname)
			
			if (curRenderer == 'Arnold Render'):
				if ArnoldWatermarked:
					block = af.Block( blockname, 'xsi_arnold_watermarked')
				else:
					block = af.Block( blockname, 'xsi_arnold')
			
			if (curRenderer == 'mental ray'):
				block = af.Block( blockname, 'xsi')
				
			if (curRenderer == 'Redshift'):
				block = af.Block( blockname, 'xsi_redshift')
				
			block.setCommand( str( blockcmds[i]))
			block.setFiles( blockimages[i])
			block.setNumeric( cp_frame_start, cp_frame_end, frame_fpt, cp_frame_by)

			if capacity   != -1: block.setCapacity( capacity)
			if maxruntime !=  0: block.setTasksMaxRunTime( int( maxruntime * 3600))
			
			job.blocks.append( block)
			i += 1

		
		# Send job:
		if not job.send()[0]:
			Application.LogMessage('Error: Can\'t send job to server.')
			os.remove(tmpscene)
