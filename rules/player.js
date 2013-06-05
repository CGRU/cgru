p_PLAYER = true;

p_savepath = '.commented';

p_imgTypes = ['jpg','jpeg','png'];

p_path = null;
p_imageMode = false;
p_elImg = [];
p_images = [];
p_frame = null;
p_playing = null;
p_timer = null;
p_filenames = null;
p_fileObjs = null;
p_fileSizeTotal = null;
p_fileSizeLoaded = null;
p_loadStartMS = null;
p_loaded = false;
p_numloaded = 0;
p_top = '38px';
p_bottom = '50px';

p_slidering = false;
p_slidertimer = null;

p_view_tx = 0;
p_view_ty = 0;
p_view_dx = 10;
p_view_dy = 10;
p_view_zoom = 1;
p_view_dz = .1;

p_painting = false;
p_paintElCanvas = [];
p_paintColor = [255,255,0];
p_paintSize = 8;
p_paintCtx = null;

p_bar_clr_canvas = [ 255, 255, 0];
p_bar_clr_edited = [ 255,   0, 0];
p_bar_clr_saved  = [   0, 255, 0];
p_frame_bar_height = 16;
p_frame_bar_width = 2000;

p_commenting = false;
p_comments = [];
p_cm_saved = [];
p_cm_keytime = (new Date()).getTime();

p_saving = false;
p_filestosave = 0;
p_filessaved = 0;

p_fps = 24.0;
p_interval = 40;
p_drawTime = new Date();

p_elements = ['player_content','play_slider','frame_bar','view','framerate'];
p_el = {};
p_buttons = ['play','prev','next','reverse','rewind','forward'];
p_elb = {};

g_auth_user = null;

function p_OpenCloseHeader(){u_OpenCloseHeaderFooter($('headeropenbtn'),'header',-200,0);}
function p_OpenCloseFooter(){u_OpenCloseHeaderFooter($('footeropenbtn'),'footer',50,250);}

function p_Init()
{
	cgru_Init();
	u_Init();
	c_Init();

	for( var i = 0; i < p_elements.length; i++)
		p_el[p_elements[i]] = document.getElementById( p_elements[i]);

	for( var i = 0; i < p_buttons.length; i++)
		p_elb[p_buttons[i]] = document.getElementById('btn_'+p_buttons[i]);

	var request = {};
	var config = c_Parse( n_Request({"initialize":request}));
	if( config == null ) return;
	for( var file in config.config )
		cgru_ConfigJoin( config.config[file].cgru_config );
	if( config.user )
		g_auth_user = config.user;

	c_RulesMergeDir( RULES, n_WalkDir(['.'], 0, RULES.rufolder, ['rules'])[0]);

	if( RULES.cgru_config )
		cgru_ConfigJoin( RULES.cgru_config );

	if( localStorage.player_precreate == null ) localStorage.player_precreate = 'OFF';
	document.getElementById('player_precreate').textContent = localStorage.player_precreate;
	if( localStorage.player_usewebgl == null ) localStorage.player_usewebgl = 'ON';
	if( localStorage.player_usewebgl == 'ON' )
	{
		gl_Init();
		if( gl == null )
		{
			localStorage.player_usewebgl = 'OFF';
			alert('Unable to initialize WebGL. Your browser may not support it.\nIt is disabled now, 2d canvas will be used.\nYou can open settings in upper left corner.');
		}
	}
	document.getElementById('player_usewebgl').textContent = localStorage.player_usewebgl;

	u_DrawColorBars( $('paint_palette'), p_ColorOnClick, 25);
	p_el.view.onmousedown = p_ViewOnMouseDown;
	p_el.view.onmousemove = p_ViewOnMouseMove;
	p_el.view.onmouseup = p_ViewOnMouseUp;
	p_el.view.onmouseover = p_ViewOnMouseOver;

	p_el.frame_bar.onmousedown = p_SliderOnMouseDown;
	p_el.frame_bar.onmouseup = p_SliderOnMouseUp;
	p_el.frame_bar.onmousemove = p_SliderOnMouseMove;
	p_el.frame_bar.onmouseout = p_SliderOnMouseOut;
	document.body.onkeydown = p_OnKeyDown;
	window.onhashchange = p_PathChanged;
//	window.onresize = p_ViewHome;

	$('paint_size_num').onblur = function(e){ p_PaintSizeSet()};
	$('paint_size_num').onkeydown = p_PaintSizeKeyDown;
	p_PaintColorSet( p_paintColor);
	p_PaintSizeSet( p_paintSize);

	$('comments').onkeydown = p_CmOnKeyDown;
	$('comments_body').onblur = p_CmProcess;

	p_PathChanged();
}

function p_Info( i_text)
{
	$('info_panel').innerHTML = i_text;
}

function p_PrecreateOnClick()
{
	if( localStorage.player_precreate == 'ON' ) localStorage.player_precreate = 'OFF';
	else localStorage.player_precreate = 'ON';
	document.getElementById('player_precreate').textContent = localStorage.player_precreate;
	p_Deactivate();
}
function p_UseWebGLOnClick()
{
	if( localStorage.player_usewebgl == 'ON' ) localStorage.player_usewebgl = 'OFF';
	else localStorage.player_usewebgl = 'ON';
	document.getElementById('player_usewebgl').textContent = localStorage.player_usewebgl;
	p_Deactivate();
}

function p_Deactivate()
{
	p_loaded = false;
	p_StopTimer();
	for( var btn in p_elb ) p_elb[btn].style.display = 'none';
	c_Info('DEACTIVATED. Need to be restarted (F5).');
}

function p_PathChanged()
{
	p_loaded = false;
	p_numloaded = 0;
	p_PushAllButtons();

	p_StopTimer();
	p_frame = 0;
	p_playing = 0;

	p_images = [];
	p_paintElCanvas = [];

	p_path = c_GetHashPath();

	var walk = n_WalkDir([p_path], 0, '.rules',['player'])[0];
	c_RulesMergeDir( RULES, walk);
	RULES.rufiles = walk.rufiles;
	if( walk.files == null )
	{
		if( c_FileCanEdit( p_path))
		{
			walk.files = [{"name":c_PathBase( p_path)}];
			p_path = c_PathDir( p_path);
			p_imageMode = true;
			$('playback_controls').style.display = 'none';
		}
		else
		{
			c_Error('Cant`t play ' + p_path);
			return;
		}
	}

	p_filenames = [];
	p_fileObjs = {};
	p_fileSizeTotal = 0;
	p_fileSizeLoaded = 0;
	p_loadStartMS = (new Date()).valueOf();

	walk.files.sort( c_CompareFiles );

	for( var i = 0; i < walk.files.length; i++)
	{
		var file = walk.files[i].name;
		p_fileObjs[file] = walk.files[i];
		if( walk.files[i].size )
			p_fileSizeTotal += walk.files[i].size;
		var type = file.split('.').pop().toLowerCase();
		if( p_imgTypes.indexOf( type ) == -1 ) continue;
		var img = new Image();
		img.src = RULES.root + p_path + '/' + file;
		img.onload = function(e){p_ImgLoaded(e);}
		img.onerror = function(e){p_ImgLoaded(e);}
		img.m_file = walk.files[i];
		p_filenames.push( file)
		p_images.push( img);
	}

	if( p_filenames == null || ( p_filenames.length == 0 ))
	{
		if( walk.error )
			c_Error( walk.error);
		else
			c_Error('No JPEG or PNG Files Founded.');
		return;
	}

	if( p_imageMode )
	{
		p_savepath = p_path;
	}
	else
	{
		var path = p_path.substr( 0, p_path.lastIndexOf('/'));
		var folder = p_path.substr( p_path.lastIndexOf('/')+1);
		p_savepath = path+'/'+folder+p_savepath;
	}

	c_Info('Loading '+p_images.length+' images: '+c_Bytes2KMG( p_fileSizeTotal));
	p_Info('Loading images sequence');

	window.document.title = p_path.substr( p_path.lastIndexOf('/')+1)+'/'+p_filenames[0];
}

function p_ImgLoaded(e)
{
	var img = e.currentTarget;
	p_numloaded++;
	if( img.m_file && img.m_file.size ) p_fileSizeLoaded += img.m_file.size;

	var info = 'Loaded '+p_numloaded+' of '+p_filenames.length+' images';
	info += ': '+c_Bytes2KMG( p_fileSizeLoaded)+' of '+c_Bytes2KMG( p_fileSizeTotal);

	var sec = ((new Date()).valueOf() - p_loadStartMS) / 1000;
	if( sec > 0 )
	{
		var speed = p_fileSizeLoaded / sec;
		info += ': '+c_Bytes2KMG( speed)+'/s';
	}

	c_Info( info, false);
	p_el.play_slider.style.width = Math.round(100.0*p_numloaded/p_filenames.length) + '%';

	if( p_numloaded < p_filenames.length ) return;

	p_loaded = true;
	p_PushButton();

	// Initialize frames bar canvas context:
	$('bar_canvas').width = p_frame_bar_width;
	$('bar_canvas').height = p_frame_bar_height;
	p_bar_ctx = $('bar_canvas').getContext('2d');
	p_bar_ctx.globalCompositeOperation = 'source-over';

	if( localStorage.player_usewebgl == 'ON' )
	{
		p_Info('Starting web GL');
		gl_Start();
	}
	else
	{
		p_Info('Creating images');
		p_CreateImages();
	}

	p_ShowFrame( p_frame);
	p_ViewHome();
//	setTimeout('p_ViewHome();',100);

	// Just information:
	var info = p_images.length+' images '+p_images[0].width+'x'+p_images[0].height;
	if( p_fileSizeTotal ) info += ': loaded '+c_Bytes2KMG( p_fileSizeTotal);
	info += ' at '+sec.toFixed(1)+' seconds';
	if(( sec > 0 ) && p_fileSizeTotal )
	{
		var speed = p_fileSizeLoaded / sec;
		info += ': '+c_Bytes2KMG( speed)+'/s';
	}
	p_Info( info);

	// Process saved comments:
	var walk = n_WalkDir([p_savepath], 0, '.rules',['player'])[0];
	c_RulesMergeDir( RULES, walk);
	if( RULES.player && RULES.player.comments )
	{
		for( var f = 0; f < p_filenames.length; f++)
		{
			var cm = RULES.player.comments[p_filenames[f]];
			if( cm )
			{
				p_comments[f] = cm.text;
				p_cm_saved[f] = true;
			}
		}
	}

	// Process saved painted images:
	if( RULES.rufiles && RULES.rufiles.length )
	for( var f = 0; f < p_filenames.length; f++ )
	{
		var pngname = p_filenames[f] + '.png';
		if( RULES.rufiles.indexOf( pngname ) == -1 ) continue;
		p_png = new Image();
		p_png.src = RULES.root + p_path + '/.rules/' + pngname;
		p_png.m_frame = f;
		p_png.onload = function(e) {
			var img = e.currentTarget;
			var canvas = p_PaintCreateCanvas( img.m_frame);
			if( img.m_frame != p_frame ) canvas.style.display = 'none';
			canvas.m_edited = true;
			canvas.m_saved = true;
			var ctx = canvas.getContext('2d');
			ctx.globalCompositeOperation = 'destination-over';
			ctx.drawImage( img, 0, 0);
			p_SetEditingState( true);
		}
	}

	p_ShowFrame( p_frame);
	p_SetEditingState( true);
}

function p_CreateImages()
{
	for( var i = 0; i < p_elImg.length; i++)
		p_el.view.removeChild( p_elImg[i]);
	p_elImg = [];

	var imgnum = 1;
	if( localStorage.player_precreate == 'ON' ) imgnum = p_images.length;
	for( var i = 0; i < imgnum; i++)
	{
		var elImg = document.createElement('img');
		p_el.view.appendChild( elImg);
		elImg.src = RULES.root + p_path + '/' + p_filenames[i];
		if( localStorage.player_precreate == 'ON' )
			elImg.style.display = 'none';
		elImg.onmousedown = function(){return false;}
		p_elImg.push( elImg);
	}
}

function p_ViewHome()    { p_ViewTransform( 0, 0, 1); }
function p_ViewLeft()    { p_ViewTransform( p_view_tx - p_view_dx, p_view_ty, p_view_zoom ); }
function p_ViewRight()   { p_ViewTransform( p_view_tx + p_view_dx, p_view_ty, p_view_zoom ); }
function p_ViewUp()      { p_ViewTransform( p_view_tx, p_view_ty - p_view_dy, p_view_zoom ); }
function p_ViewDown()    { p_ViewTransform( p_view_tx, p_view_ty + p_view_dy, p_view_zoom ); }
function p_ViewZoomIn()  { p_ViewTransform( p_view_tx, p_view_ty, ( 1.0 + p_view_dz ) * p_view_zoom ); }
function p_ViewZoomOut() { p_ViewTransform( p_view_tx, p_view_ty, ( 1.0 - p_view_dz ) * p_view_zoom ); }

function p_ViewTransform( i_tx, i_ty, i_zoom)
{
//console.log( 'p_ViewTransform: ' + i_tx + ',' + i_ty + 'x' + i_zoom);
//console.log(p_elImg[0].width+' x '+p_elImg[0].height);
//console.log(p_el.player_content.clientWidth+' x '+p_el.player_content.clientHeight);
	if( i_zoom <= 0 ) return;

	// If we are near 1.0 we set to 1 (no zoom)
	if( Math.abs( 1.0 - i_zoom ) < ( .7 * p_view_dz ))
		i_zoom = 1;

	p_view_zoom = i_zoom;
	p_view_tx = i_tx;
	p_view_ty = i_ty;

	var ml = 0, mt = 0;
	var img_w = p_images[0].width;
	var img_h = p_images[0].height;
	if(( img_w > 0 ) && ( img_h > 0 ))
	{
		var wnd_w = p_el.player_content.clientWidth;
		var wnd_h = p_el.player_content.clientHeight;
		ml = Math.round((wnd_w - img_w) / 2);
		mt = Math.round((wnd_h - img_h) / 2);
	}
	ml += p_view_tx;
	mt += p_view_ty;

	p_el.view.style.width = img_w + 'px';
	p_el.view.style.height = img_h + 'px';
	p_el.view.style.marginLeft = ml + 'px';
	p_el.view.style.marginTop = mt + 'px';

	if( p_view_zoom === 1 )
	{
		$('view_zoom').classList.remove('zoomed');
		$('view_zoom').textContent = 'x1';
		p_el.view.style.transform = '';
	}
	else
	{
		$('view_zoom').classList.add('zoomed');
		$('view_zoom').textContent = 'x' + p_view_zoom.toFixed(2);
		p_el.view.style.transform = 'scale('+p_view_zoom+','+p_view_zoom+')';
	}
}

function p_OnKeyDown(e)
{
//window.console.log(e.keyCode);
	if( e.keyCode == 116) // F5
	{
		c_Info('Use CTRL+R to refresh. All loaded and painted images will be lost!');
		return false;
	}

	if( e.ctrlKey ) return;
	if( e.altKey  ) return;

	if( e.keyCode == 27 ) // ESC
	{
		cgru_ClosePopus();
		p_ShowFrame(0);
		p_ViewHome();
	}

	else if( e.keyCode == 33  ) p_NextFrame(-10); // PageUp
	else if( e.keyCode == 34  ) p_NextFrame(+10); // PageDown
	else if( e.keyCode == 35  ) p_ShowFrame(-1);  // End
	else if( e.keyCode == 36  ) p_ShowFrame(0);   // Home
	else if( e.keyCode == 219 ) p_NextFrame(-1);  // [
	else if( e.keyCode == 221 ) p_NextFrame(+1);  // ]
	else if( e.keyCode == 32  ) p_Play();         // Space
	else if( e.keyCode == 190 ) p_Play();         // >
	else if( e.keyCode == 82  ) p_Reverse();      // R
	else if( e.keyCode == 188 ) p_Reverse();      // <

	else if( e.keyCode == 39  ) p_ViewRight();    // Right
	else if( e.keyCode == 37  ) p_ViewLeft();     // Left
	else if( e.keyCode == 38  ) p_ViewUp();       // Up
	else if( e.keyCode == 40  ) p_ViewDown();     // Down
	else if( e.keyCode == 173 ) p_ViewZoomOut();  // -
	else if( e.keyCode == 109 ) p_ViewZoomOut();  // - (NumPad)
	else if( e.keyCode == 61  ) p_ViewZoomIn();   // + 
	else if( e.keyCode == 107 ) p_ViewZoomIn();   // + (NumPad)
	else if( e.keyCode == 72  ) p_ViewHome();     // H
	else if( e.keyCode == 70  ) p_FullScreen();   // F

	else if( e.keyCode == 80  ) p_Paint();             // P
	else if( e.keyCode == 57  ) p_PaintSizeChange(-1); // 9
	else if( e.keyCode == 48  ) p_PaintSizeChange(+1); // 0

	else if( e.keyCode == 67  ) p_Comment(); // C

	else if( e.keyCode == 59  ) p_NextEditedFrame(-1); // ;
	else if( e.keyCode == 222 ) p_NextEditedFrame(+1); // '

	else if( e.keyCode == 83  ) p_Save(); // S
}

function p_FullScreen()
{
	if( $('header').m_hidden )
	{
		$('header').m_hidden = false;
		$('header').style.display = 'block';
		$('footer').style.display = 'block';
//p_el.player_content.style.top = p_top;
//p_el.player_content.style.bottom = p_bottom;
	}
	else
	{
		$('header').m_hidden = true;
		$('header').style.display = 'none';
		$('footer').style.display = 'none';
//p_el.player_content.style.top = '0';
//p_el.player_content.style.bottom = '0';
	}
//p_ViewHome();
}

function p_PushButton( i_btn)
{
	for( var btn in p_elb )
		p_elb[btn].classList.remove('pushed');
	if( i_btn)
		p_elb[i_btn].classList.add('pushed');
}
function p_PushAllButtons()
{
	for( var btn in p_elb )
		p_elb[btn].classList.add('pushed');
}

function p_Play()
{
	if( p_imageMode ) return;
	if( p_loaded == false ) return;
	if( p_playing > 0 )
	{
		p_Pause();
		return;
	}
	p_PushButton('play');
	p_playing = 1;
	p_interval = Math.round(1000/p_fps);
	p_NextFrame();
}

function p_Reverse()
{
	if( p_imageMode ) return;
	if( p_loaded == false ) return;
	if( p_playing < 0 )
	{
		p_Pause();
		return;
	}
	p_PushButton('reverse');
	p_playing = -1;
	p_interval = Math.round(1000/p_fps);
	p_NextFrame();
}

function p_Pause()
{
	if( p_loaded == false ) return;
	p_StopTimer();
	if( p_playing == 0 ) return;
	p_PushButton();
	p_playing = 0;
}

function p_StopTimer()
{
	if( p_timer ) clearInterval( p_timer);
}

function p_Rewind( i_dir)
{
	if( p_loaded == false ) return;
	if( i_dir )
		p_ShowFrame( p_images.length - 1);
	else
		p_ShowFrame( 0);
}

function p_ShowFrame( i_val)
{
	if( p_loaded == false ) return;
	if( i_val == -1 )
		i_val = p_images.length - 1;
	p_NextFrame( i_val - p_frame );
}

function p_NextFrame( i_val)
{
	if( p_loaded == false ) return;

	if( localStorage.player_usewebgl == 'OFF' )
		if( localStorage.player_precreate == 'ON' )
			p_elImg[p_frame].style.display = 'none';

	if( p_paintElCanvas[p_frame])
		p_paintElCanvas[p_frame].style.display = 'none';

	if( i_val )
	{
		p_StopTimer();
		p_playing = 0;
		p_PushButton();
		p_frame += i_val;
	}
	else
	{
		p_frame += p_playing;
	}

	if( p_frame >= p_images.length ) p_frame = 0;
	if( p_frame < 0 ) p_frame = p_images.length - 1;

	if( localStorage.player_usewebgl == 'ON' )
		gl_DrawScene();
	else
	{
		if( localStorage.player_precreate == 'ON' )
			p_elImg[p_frame].style.display = 'block';
		else
			p_elImg[0].src = RULES.root + p_path + '/' + p_filenames[p_frame];
	}

	if( p_paintElCanvas[p_frame])
		p_paintElCanvas[p_frame].style.display = 'block';

	c_Info( p_filenames[p_frame], false);
	var width = '100%';
	if( p_images.length > 1 )
		width = 100.0*p_frame/(p_images.length-1) + '%';
	p_el.play_slider.style.width = width;

	p_SetEditingState();

	var now = new Date();
	var delta = now.valueOf() - p_drawTime.valueOf();
	var fps = 1000 / delta;
//	p_el.framerate.textContent = (''+fps).substr(0,5);
	p_el.framerate.textContent = Math.round( fps);
	p_drawTime = now;
	if( fps < p_fps)
	{
		 if( p_interval > 0 ) p_interval--;
//		 p_interval--;
	}
	else p_interval++;

	if( p_playing != 0 )
	{
		p_StopTimer();
		p_timer = setTimeout('p_NextFrame()', p_interval);
	}
}

function p_NextEditedFrame( i_dir)
{
	if( p_loaded == false ) return;
	f = p_frame + i_dir;
	while(( p_paintElCanvas[f] == null ) && ( p_comments[f] == null ))
	{
		if(( f < 0 ) || ( f >= p_images.length )) return;
		f += i_dir;
	}
	p_ShowFrame( f);
}

function p_SliderOnMouseDown( i_evt)
{
//window.console.log('p_SliderOnMouseDown');
	i_evt.stopPropagation();
	if( p_loaded == false ) return;
	p_slidering = true;
	p_SliderOnMouseMove( i_evt);
	return false;
}
function p_SliderOnMouseMove( i_evt)
{
//window.console.log('p_SliderOnMouseMove');
	i_evt.stopPropagation();
	if( p_slidertimer ) clearInterval( p_slidertimer);
	if( p_loaded == false ) return;
	if( false == p_slidering ) return;
	var width = p_el.frame_bar.clientWidth - 1;
	var offset = i_evt.clientX - p_el.frame_bar.offsetLeft;
//	p_el.play_slider.style.width = 100.0*offset/width + '%';
	p_ShowFrame( Math.floor( p_images.length * offset / width ));
	return false;
}
function p_SliderOnMouseUp()
{
//window.console.log('p_SliderOnMouseUp');
	p_slidering = false;
}
function p_SliderOnMouseOut()
{
//window.console.log('p_SliderOnMouseOut');
	if( p_slidertimer ) clearInterval( p_slidertimer);
	p_slidertimer = setTimeout('p_slidering = false;', 1000);
}

function p_Save()
{
	if( g_auth_user == null )
	{
		c_Error('Guests can`t save.');
		return;
	}

	if( p_saving ) return;

	p_saving = true;
	$('save_btn').classList.add('pushed');

	p_filestosave = 0;
	p_filessaved = 0;

	p_CommentsSave();
	p_PaintSave();
}

function p_SetEditingState( i_update_whole_bar )
{
	p_PaintSetState();
	p_CmSetState();

	// Draw frame bar:
	var start_frame = p_frame;
	var end_frame = p_frame;
	if( i_update_whole_bar )
	{
		start_frame = 0;
		end_frame = p_images.length-1;
	}
	var width = p_frame_bar_width / (p_images.length-1);
	for( var f = start_frame; f <= end_frame; f++)
	{
		var pos = p_frame_bar_width * f / (p_images.length-1) - .5*width;

		if( p_paintElCanvas[f] || p_comments[f] )
		{
			var color = p_bar_clr_canvas;
			if(( p_paintElCanvas[f] && p_paintElCanvas[f].m_edited ) || p_comments[f] )
			{
				color = p_bar_clr_edited;
				if(( p_paintElCanvas[f] && p_paintElCanvas[f].m_saved && p_cm_saved[f] ) ||
					(( p_paintElCanvas[f] == null ) && p_cm_saved[f] ) ||
					(( p_comments[f] == null ) && p_paintElCanvas[f].m_saved ))
					color = p_bar_clr_saved;
			}
			p_bar_ctx.fillStyle = 'rgb('+color.join(',')+')';
			p_bar_ctx.fillRect( pos, 0, width, p_frame_bar_height);
		}
		else
			p_bar_ctx.clearRect( pos, 0, width, p_frame_bar_height);
	}
}

function p_Paint()
{
	if( p_painting )
	{
		$('info_panel').style.display = 'block';
		$('paint_panel').style.display = 'none';
		$('paint_btn').classList.remove('pushed');
		p_painting = false;
	}
	else
	{
		$('info_panel').style.display = 'none';
		$('paint_panel').style.display = 'block';
		$('paint_btn').classList.add('pushed');
		p_painting = true;
	}
}

function p_ViewOnMouseDown( i_evt)
{
i_evt.stopPropagation();
	if( p_loaded == false ) return;
	if( p_painting == false ) return;

	var canvas = null;
	if( p_paintElCanvas[p_frame] )
		canvas = p_paintElCanvas[p_frame];
	else
		canvas = p_PaintCreateCanvas();

//var c = p_GetCtxCoords( i_evt);
	p_paintCtx = canvas.getContext('2d');
	p_paintCtx.globalCompositeOperation = 'source-over';
	p_paintCtx.beginPath();
	p_paintCtx.lineWidth = p_paintSize;
	p_paintCtx.lineCap = 'round';
	p_paintCtx.strokeStyle = 'rgb('+p_paintColor.join(',')+')';

	p_SetEditingState();
}
function p_PaintCreateCanvas( i_frame)
{
	if( i_frame == null ) i_frame = p_frame;
	var canvas = document.createElement('canvas');
	p_paintElCanvas[i_frame] = canvas;
	p_el.view.appendChild( canvas);
	canvas.width = p_images[i_frame].width;
	canvas.height = p_images[i_frame].height;
	canvas.style.width = canvas.width + 'px';
	canvas.style.height = canvas.height + 'px';
	c_Info('Canvas for "'+p_filenames[i_frame]+'" created.');
	return canvas;
}
function p_GetCtxCoords( i_evt)
{
/*
window.console.log('e['+i_evt.clientX+','+i_evt.clientY+']'+
' t['+i_evt.currentTarget.offsetLeft+','+i_evt.currentTarget.offsetTop+']'+
' v['+p_el.view.offsetLeft+','+p_el.view.offsetTop+']'+
' c['+p_el.player_content.offsetLeft+','+p_el.player_content.offsetTop+']'+
'');
*/
	var c = {};
	c.x = i_evt.clientX - p_el.view.offsetLeft - p_el.player_content.offsetLeft;
	c.y = i_evt.clientY - p_el.view.offsetTop - p_el.player_content.offsetTop;
	if( p_view_zoom !== 1 )
	{
		var img_w = p_images[0].width;
		var img_h = p_images[0].height;
		c.x -= 0.5 * img_w;
		c.y -= 0.5 * img_h;
		c.x /= p_view_zoom;
		c.y /= p_view_zoom;
		c.x += 0.5 * img_w;
		c.y += 0.5 * img_h;
		c.x = Math.round( c.x);
		c.y = Math.round( c.y);
	}
//console.log(c);
	return c;
}
function p_ViewOnMouseMove( i_evt)
{
	if( p_paintCtx == null ) return;
	var c = p_GetCtxCoords( i_evt);
	p_paintCtx.lineTo( c.x, c.y);
	p_paintElCanvas[p_frame].m_edited = true;
	p_paintElCanvas[p_frame].m_saved = false;
	p_paintCtx.stroke();
	p_SetEditingState();
}
function p_ViewOnMouseUp()
{
	p_paintCtx = null;
}
function p_ViewOnMouseOver()
{
	p_paintCtx = null;
}

function p_ColorOnClick( e)
{
	var clrEl = e.currentTarget;
	el = clrEl.parentNode.parentNode.parentNode;
	p_PaintColorSet(clrEl.m_color);
}
function p_PaintColorSet( i_clr)
{
	if( i_clr == null )
		return;
	$('paint_color').style.background = 'rgb(' + i_clr.join(',') + ')';
	p_paintColor = i_clr;
}

function p_PaintSizeSet( i_px)
{
	if( i_px == null )
	{
		i_px = parseInt( $('paint_size_num').textContent);
	}
	if( false == isNaN( i_px))
	{
		p_paintSize = i_px;
	}
	
	if( p_paintSize < 1 ) p_paintSize = 1;
	$('paint_size_num').textContent = p_paintSize;
}
function p_PaintSizeChange( i_px) { if( p_painting ) p_PaintSizeSet( p_paintSize + i_px); }
function p_PaintSizeKeyDown( e)
{
	if( e.keyCode == 13 ) return false; // Enter
	if( e.keyCode == 38 ) { p_PaintSizeChange( 1); return false; }; // UP
	if( e.keyCode == 40 ) { p_PaintSizeChange(-1); return false; }; // DOWN
}

function p_PaintClear()
{
	if( p_loaded == false ) return;
	if( p_painting == false ) return;

	var canvas = p_paintElCanvas[p_frame];
	if( canvas == null ) return;

	p_paintElCanvas[p_frame] = null;
	p_el.view.removeChild( canvas);

	p_SetEditingState();
}

function p_PaintSave()
{
	for( var f = 0; f < p_images.length; f++)
	{
		var canvas = p_paintElCanvas[f];
		var canvas_dummy = false;
		if( canvas == null )
		{
			if( p_comments[f] == null )
				continue;
			canvas = document.createElement("canvas");
			canvas.width = p_images[f].width;
			canvas.height = p_images[f].height;
			canvas.m_edited = true;
			canvas_dummy = true;
		}
		if( canvas.m_edited != true ) continue;
		if( canvas.m_saved ) continue;

		p_filestosave++;

		var path = RULES.root + p_savepath + '/' + p_filenames[f];
		if( p_imageMode )
			path += '.painted.jpg';
		else if( true != canvas_dummy )
		{
			var png = canvas.toDataURL('image/png');
			png = png.substr( png.indexOf(',')+1);
			var png_path = RULES.root + p_path + '/.rules/' + p_filenames[f] + '.png';
			p_filestosave++;
			n_Request({"save":{"file":png_path,"data":png,"type":"base64"}}, false);
		}

		var ctx = canvas.getContext('2d');
		ctx.globalCompositeOperation = 'destination-over';
		ctx.drawImage( p_images[f], 0, 0);
		var data = canvas.toDataURL('image/jpeg',.8);
		data = data.substr( data.indexOf(',')+1);

		n_Request({"save":{"file":path,"data":data,"type":"base64"}}, false);
	}

	if( p_filestosave == 0 )
		p_SavingFinished();
}

function n_MessageReceived( i_msg)
{
//window.console.log(JSON.stringify(i_msg));
	if( i_msg.error )
	{
		c_Error( i_msg.error);
		return;
	}
	var file =  i_msg.save;
	if( file )
	{
		p_filessaved++;
		var name = file.substr( file.lastIndexOf('/')+1);
		var frame = p_filenames.indexOf(name);
		if( p_imageMode ) frame = 0;
		if( p_paintElCanvas[frame])
			p_paintElCanvas[frame].m_saved = true;
//p_SetEditingState();
		c_Info('Saved "'+name +'" '+p_filessaved+' of '+p_filestosave);
		if( p_filessaved >= p_filestosave )
			p_SavingFinished( file.substr( 0, file.lastIndexOf('/')).substr( file.indexOf('/')));
	}
}

function p_SavingFinished( folder)
{
	p_saving = false;
	$('save_btn').classList.remove('pushed');
	if( folder && p_filessaved )
		c_Info('Saved '+p_filessaved+' images to "'+folder+'"');
	p_SetEditingState( true);
}

function p_PaintSetState()
{
	var shadow = '0 0 4px #000';
	if( p_paintElCanvas[p_frame] )
	{
		shadow = '0 0 4px #FF0';
		if( p_paintElCanvas[p_frame].m_edited )
		{
			shadow = '0 0 4px #F00';
			if( p_paintElCanvas[p_frame].m_saved )
				shadow = '0 0 4px #0F0';
		}
	}

	p_el.view.style.boxShadow = shadow;
	$('paint_btn').style.boxShadow = shadow;
}

// ===================== Comments: ===================

function p_Comment()
{
	if( p_commenting )
	{
		$('comments').style.display = 'none';
		$('comments_btn').classList.remove('pushed');
		p_commenting = false;
	}
	else
	{
		$('comments').style.display = 'block';
		$('comments_btn').classList.add('pushed');
		p_commenting = true;
	}
}

function p_CmOnKeyDown( e) { e.stopPropagation(); }

function p_CmProcess()
{
	var text = $('comments_body').innerHTML;
	if( text.length )
	{
		if( p_comments[p_frame] == text )
			return;
		p_comments[p_frame] = text;
	}
	else
		p_comments[p_frame] = null;

	p_cm_saved[p_frame] = false;
	p_SetEditingState();
}

function p_CmSetState()
{
	var shadow = '0 0 4px #000';
	if( p_comments[p_frame] )
	{
		shadow = '0 0 4px #F00';
		$('comments_body').innerHTML = p_comments[p_frame];
		if( p_cm_saved[p_frame] )
			shadow = '0 0 4px #0F0';
	}
	else
		$('comments_body').innerHTML = '';
	$('comments_btn').style.boxShadow = shadow;
}

function p_CommentsSave()
{
	var pcms = {};

	var rcm = {};
	rcm.user_name = g_auth_user.id;
	rcm.ctime = (new Date()).getTime();
	rcm.sequence = p_path;
	rcm.frames = [];

	for( var f = 0; f < p_images.length; f++ )
	{
		if( p_comments[f] == null ) continue;
		if( p_cm_saved[f] ) continue;
		p_cm_saved[f] = true;

		var fcm = {};
		fcm.file = p_filenames[f];
		fcm.text = p_comments[f];
		rcm.frames.push( fcm);

		var pcm = {};
		pcm.ctime = rcm.ctime;
		pcm.cuser = rcm.cuser;
		pcm.text = p_comments[f];
		pcms[p_filenames[f]] = pcm;
	}

	if( rcm.frames.length == 0 )
		return;

	var key = p_cm_keytime + '_' + g_auth_user.id;
	var edit = {};
	edit.add = true;

	edit.object = {"player":{"comments":pcms}};
	edit.file = RULES.root + p_savepath + '/' + RULES.rufolder + '/player.json';
	var res = c_Parse( n_Request({"editobj":edit}));
	if( c_NullOrErrorMsg( res)) return;

	var comments = {};
	comments[key] = rcm;
	edit.object = {"comments":comments};
	edit.file = RULES.root + p_savepath + '/' + RULES.rufolder + '/comments.json';
	var res = c_Parse( n_Request({"editobj":edit}));
	if( c_NullOrErrorMsg( res)) return;

	c_Info('Comments saved.');

console.log(JSON.stringify( rcm));
}

// ====================================================
// ====================== WEB GL ======================
// ====================================================

var gl;

var gl_elCanvas;
var gl_textures = [];

var gl_vtxBuf;
var gl_vtxUVBuf;
var gl_vtxIndexBuf;

var gl_camMatrix;
var gl_objMatrix;
var gl_shaderProgram;
var gl_vtxPosAttr;
var gl_uvAttr;

var gl_fragmentShaderSource = 'varying highp vec2 vTextureCoord; uniform sampler2D uSampler; void main(void) { gl_FragColor = texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));}';
var gl_vertexShaderSource = 'attribute vec3 aVertexPosition; attribute vec2 aTextureCoord; uniform mat4 uMVMatrix; uniform mat4 uPMatrix; varying highp vec2 vTextureCoord;	void main(void) {gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0); vTextureCoord = aTextureCoord;}';

function gl_Init()
{
	gl = null;

	gl_elCanvas = document.createElement('canvas');
	gl_elCanvas.width = 64;
	gl_elCanvas.height = 64;
	p_el.view.appendChild( gl_elCanvas);

	try
	{
		gl = gl_elCanvas.getContext("webgl") || gl_elCanvas.getContext("experimental-webgl");
	}
	catch(e)
	{
		p_el.view.removeChild( gl_elCanvas);
	}
}

function gl_Start()
{
	if( gl == null )
	{
		c_Error('GL is not created.');
		return;
	}

	gl_elCanvas.width = p_images[0].width;
	gl_elCanvas.height = p_images[0].height;
	gl.viewport( 0, 0, p_images[0].width, p_images[0].height);

	gl.clearColor(0.0, 0.0, 0.0, 1.0);  // Clear to black, fully opaque
	gl.clearDepth(1.0);                 // Clear everything
	gl.enable(gl.DEPTH_TEST);           // Enable depth testing
	gl.depthFunc(gl.LEQUAL);            // Near things obscure far things

	// Ortho camera projection
	gl_camMatrix = new Float32Array(	[1, 0, 0, 0,
										 0, 1, 0, 0,
										 0, 0, 0, 0,
										 0, 0, 0, 1]);
	// Object space
	gl_objMatrix = new Float32Array(	[1, 0, 0, 0,
										 0, 1, 0, 0,
										 0, 0, 1, 0,
										 0, 0, 0, 1]);

	gl_InitShaders();
	gl_InitBuffers();
	gl_InitTextures();

//	setInterval("drawScene()", 40);
}

function gl_InitShaders()
{
	// Create the shader program
	gl_shaderProgram = gl.createProgram();
	gl.attachShader( gl_shaderProgram, gl_InitShader( gl_vertexShaderSource, gl.VERTEX_SHADER));
	gl.attachShader( gl_shaderProgram, gl_InitShader( gl_fragmentShaderSource, gl.FRAGMENT_SHADER));
	gl.linkProgram( gl_shaderProgram);

	// If creating the shader program failed, alert
	if (!gl.getProgramParameter( gl_shaderProgram, gl.LINK_STATUS))
		c_Error('Unable to initialize the shader program.');

	gl.useProgram( gl_shaderProgram);
	gl_vtxPosAttr = gl.getAttribLocation( gl_shaderProgram, "aVertexPosition");
	gl.enableVertexAttribArray( gl_vtxPosAttr);
	gl_uvAttr = gl.getAttribLocation( gl_shaderProgram, "aTextureCoord");
	gl.enableVertexAttribArray( gl_uvAttr);
}
function gl_InitShader( i_src, i_type)
{
	var shader = gl.createShader( i_type);
	// Send the source to the shader object
	gl.shaderSource( shader, i_src);
	// Compile the shader program
	gl.compileShader( shader);
	// See if it compiled successfully
	if( !gl.getShaderParameter( shader, gl.COMPILE_STATUS))
	{
		c_Error('An error occurred compiling the shaders: ' + gl.getShaderInfoLog( shader));
		return null;
	}
	return shader;
}

function gl_InitTextures()
{
	var numtex = 1;
	if( localStorage.player_precreate == 'ON' ) numtex = p_images.length;
	for( var i = 0; i < numtex; i++)
	{
		gl_textures[i] = gl.createTexture();
		gl_InitTexture( i, i);
	}
}
function gl_InitTexture( i_tex, i_img)
{
	gl.bindTexture(gl.TEXTURE_2D, gl_textures[i_tex]);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, p_images[i_img]);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);// NEAREST
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
	gl.bindTexture(gl.TEXTURE_2D, null);
}

function gl_InitBuffers()
{
	// Create a buffer for the cube's vertices.
	gl_vtxBuf = gl.createBuffer();

	// Select the gl_vtxBuf as the one to apply vertex
	// operations to from here out.
	gl.bindBuffer(gl.ARRAY_BUFFER, gl_vtxBuf);

	// Now create an array of vertices for the cube.
	var vertices = [
		-1.0, -1.0, 0.0,
		1.0, -1.0, 0.0,
		1.0, 1.0, 0.0,
		-1.0, 1.0, 0.0 ];

	// Now pass the list of vertices into WebGL to build the shape. We
	// do this by creating a Float32Array from the JavaScript array,
	// then use it to fill the current vertex buffer.
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);

	// Map the texture onto the cube's faces.
	gl_vtxUVBuf = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, gl_vtxUVBuf);

	var textureCoordinates = [
	0.0,  1.0,
	1.0,  1.0,
	1.0,  0.0,
	0.0,  0.0 ];

	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoordinates), gl.STATIC_DRAW);

	// Build the element array buffer; this specifies the indices
	// into the vertex array for each face's vertices.
	gl_vtxIndexBuf = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, gl_vtxIndexBuf);

	// This array defines each face as two triangles, using the
	// indices into the vertex array to specify each triangle's
	// position.
	var cubeVertexIndices = [ 0, 1, 2,   0, 2, 3 ];

	// Now send the element array to GL
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(cubeVertexIndices), gl.STATIC_DRAW);
}

function gl_DrawScene()
{
	// Clear the canvas before we start drawing on it.
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	// Draw the plane by binding the array buffer to the planes's vertices
	// array, setting attributes, and pushing it to GL
	gl.bindBuffer(gl.ARRAY_BUFFER, gl_vtxBuf);
	gl.vertexAttribPointer( gl_vtxPosAttr, 3, gl.FLOAT, false, 0, 0);

	// Set the texture coordinates attribute for the vertices
	gl.bindBuffer(gl.ARRAY_BUFFER, gl_vtxUVBuf);
	gl.vertexAttribPointer( gl_uvAttr, 2, gl.FLOAT, false, 0, 0);

	// Specify the texture to map onto the faces
	gl.activeTexture( gl.TEXTURE0);
	if( localStorage.player_precreate == 'ON' )
		gl.bindTexture( gl.TEXTURE_2D, gl_textures[p_frame]);
	else
	{
		gl_InitTexture( 0, p_frame);
		gl.bindTexture( gl.TEXTURE_2D, gl_textures[0]);
	}
	gl.uniform1i( gl.getUniformLocation( gl_shaderProgram, 'uSampler'), 0);

	// Draw plane
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, gl_vtxIndexBuf);

	gl.uniformMatrix4fv( gl.getUniformLocation( gl_shaderProgram, 'uPMatrix'), false, gl_camMatrix);
	gl.uniformMatrix4fv( gl.getUniformLocation( gl_shaderProgram, 'uMVMatrix'), false, gl_objMatrix);

	gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
}

