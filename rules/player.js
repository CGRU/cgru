RULES = {};
RULES.rufolder = 'rules';
RULES_TOP = {};

cgru_params.push(['user_title','User Title', 'Coordinator', 'Enter User Title']);

p_path = null;
p_elImg = [];
p_Images = [];
p_frame = null;
p_playing = null;
p_timer = null;
p_files = null;
p_loaded = false;
p_numloaded = 0;
p_top = '38px';
p_bottom = '50px';

p_slidering = false;
p_slidertimer = null;

p_painting = false;
p_elCanvas = [];
p_color = null;
p_size = 5;
p_ctx = null;

p_saving = false;
p_filestosave = 0;
p_filessaved = 0;

p_elements = ['player_content','progress','slider','view','header','footer','paint','colors','btn_save','btn_paint'];
p_el = {};
p_buttons = ['play','prev','next','reverse','rewind','forward'];
p_elb = {};

function p_OpenCloseHeader(){u_OpenCloseHeaderFooter(document.getElementById('headeropenbtn'),'header',-200,0);}
function p_OpenCloseFooter(){u_OpenCloseHeaderFooter(document.getElementById('footeropenbtn'),'footer',50,250);}

function p_Init()
{
	cgru_Init();
	u_Init();
	c_Init();

	for( var i = 0; i < p_elements.length; i++)
		p_el[p_elements[i]] = document.getElementById( p_elements[i]);

	for( var i = 0; i < p_buttons.length; i++)
		p_elb[p_buttons[i]] = document.getElementById('btn_'+p_buttons[i]);

	n_server = '../rules.php';

	var config = n_ReadConfig();
	for( var file in config.config )
		cgru_ConfigJoin( config.config[file].cgru_config );

	c_RulesMergeDir( RULES_TOP, n_WalkDir(['.'], 0, RULES.rufolder, ['rules'])[0]);

	if( RULES_TOP.cgru_config )
		cgru_ConfigJoin( RULES_TOP.cgru_config );
//	RULES = RULES_TOP;

	u_DrawColorBars( p_el.colors, p_ColorChanged(), 25);
	p_el.view.onmousedown = p_ViewOnMouseDown;
	p_el.view.onmousemove = p_ViewOnMouseMove;
	p_el.view.onmouseup = p_ViewOnMouseUp;
	p_el.view.onmouseover = p_ViewOnMouseOver;

	p_el.slider.onmousedown = p_SliderOnMouseDown;
	p_el.slider.onmouseup = p_SliderOnMouseUp;
	p_el.slider.onmousemove = p_SliderOnMouseMove;
	p_el.slider.onmouseout = p_SliderOnMouseOut;
	document.body.onkeydown = p_OnKeyDown;
	window.onhashchange = p_PathChanged;
	window.onresize = p_HomeView;

	p_PathChanged();
}

function p_PathChanged()
{
	p_loaded = false;
	p_numloaded = 0;
	p_PushAllButtons();

	p_StopTimer();
	p_frame = 0;
	p_playing = 0;

	for( var i = 0; i < p_elImg.length; i++)
		p_el.view.removeChild( p_elImg[i]);

	p_elImg = [];
	p_Images = [];
	p_elCanvas = [];

	p_path = c_GetHashPath();
//	p_path = '../'+RULES_TOP.root + p_path;
	p_path = RULES_TOP.root + p_path;
	var walk = n_WalkDir([p_path])[0];
	p_files = walk.files;

	if( p_files == null )
	{
		if( walk.error )
			c_Error( walk.error);
		else
			c_Error('No Files Founded.');
		return;
	}

	for( var i = 0; i < p_files.length; i++)
	{
		var img = new Image();
		img.src = '../'+p_path + '/' + p_files[i];
		img.onload = function(){p_ImgLoaded();}
		img.onerror = function(){p_ImgLoaded();}
		p_Images.push( img);
	}
}

function p_ImgLoaded()
{
	p_numloaded++;
	c_Info('Loaded '+p_numloaded+' of '+p_files.length+' images', false);

	if( p_numloaded < p_files.length ) return;

	p_loaded = true;
	p_PushButton();

	for( var i = 0; i < p_files.length; i++)
	{
		var elImg = document.createElement('img');
		p_el.view.appendChild( elImg);
		elImg.src = '../' + p_path + '/' + p_files[i];
		elImg.style.display = 'none';
		elImg.onmousedown = function(){return false;}
		p_elImg.push( elImg);

		c_Info( p_files[i], false);
		var width = '100%';
		if( p_files.length > 1)
			width = Math.round(100.0*i/(p_files.length-1)) + '%';
		p_el.progress.style.width = width;
	}

	p_ShowFrame( p_frame);

	p_HomeView();
//	setTimeout('p_HomeView();',100);

	c_Info('Loaded '+p_files.length+' images '+p_elImg[0].width+'x'+p_elImg[0].height);
}

function p_HomeView()
{
//window.console.log(p_elImg[0].width+' x '+p_elImg[0].height);
//window.console.log(p_el.player_content.clientWidth+' x '+p_el.player_content.clientHeight);
	var ml = '0px', mt = '0px';
	var img_w = p_elImg[0].width;
	var img_h = p_elImg[0].height;
	if(( img_w > 0 ) && ( img_h > 0 ))
	{
		var wnd_w = p_el.player_content.clientWidth;
		var wnd_h = p_el.player_content.clientHeight;
		ml = Math.round((wnd_w - img_w) / 2)+'px';
		mt = Math.round((wnd_h - img_h) / 2)+'px';
	}
	p_el.view.style.marginLeft = ml;
	p_el.view.style.marginTop = mt;
}

function p_OnKeyDown(e)
{
//window.console.log(e.keyCode);
	if( e.keyCode == 27 ) // ESC
	{
		cgru_ClosePopus();
		p_ShowFrame(0);
	}
	else if( e.keyCode == 33 ) p_NextFrame(-10); // PageUp
	else if( e.keyCode == 34 ) p_NextFrame(10); // PageDown
	else if( e.keyCode == 35 ) p_ShowFrame(-1); // End
	else if( e.keyCode == 36 ) p_ShowFrame(0); // Home
	else if( e.keyCode == 39 ) p_NextFrame(1); // Right
	else if( e.keyCode == 37 ) p_NextFrame(-1); // Left
	else if( e.keyCode == 32 ) p_Play(); // Space
	else if( e.keyCode == 38 ) p_Play(); // Up
	else if( e.keyCode == 40 ) p_Reverse(); // Down
	else if( e.keyCode == 70 ) // F
	{
		if( p_el.header.hidden )
		{
			p_el.header.hidden = false;
			p_el.header.style.display = 'block';
			p_el.footer.style.display = 'block';
			p_el.player_content.style.top = p_top;
			p_el.player_content.style.bottom = p_bottom;
		}
		else
		{
			p_el.header.hidden = true;
			p_el.header.style.display = 'none';
			p_el.footer.style.display = 'none';
			p_el.player_content.style.top = '0';
			p_el.player_content.style.bottom = '0';
		}
		p_HomeView();
	}
	else if( e.keyCode == 83 ) p_Save(); // S
	else if( e.keyCode == 72 ) p_HomeView(); // H
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
	if( p_loaded == false ) return;
	if( p_playing > 0 )
	{
		p_Pause();
		return;
	}
	p_PushButton('play');
	p_playing = 1;
	p_NextFrame();
}

function p_Reverse()
{
	if( p_loaded == false ) return;
	if( p_playing < 0 )
	{
		p_Pause();
		return;
	}
	p_PushButton('reverse');
	p_playing = -1;
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
		p_ShowFrame( p_elImg.length - 1);
	else
		p_ShowFrame( 0);
}

function p_ShowFrame( i_val)
{
	if( p_loaded == false ) return;
	if( i_val == -1 )
		i_val = p_elImg.length - 1;
	p_NextFrame( i_val - p_frame );
}

function p_NextFrame( i_val)
{
	if( p_loaded == false ) return;
p_elImg[p_frame].style.display = 'none';

	if( p_elCanvas[p_frame])
		p_elCanvas[p_frame].style.display = 'none';

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

	if( p_frame >= p_elImg.length ) p_frame = 0;
	if( p_frame < 0 ) p_frame = p_elImg.length - 1;

p_elImg[p_frame].style.display = 'block';
//p_elImg[0].style.display = 'block';
//p_elImg[0].src = p_path + '/' + p_files[p_frame];
	if( p_elCanvas[p_frame])
		p_elCanvas[p_frame].style.display = 'block';

	c_Info( p_files[p_frame], false);
	var width = '100%';
	if( p_elImg.length > 1 )
		width = 100.0*p_frame/(p_elImg.length-1) + '%';
	p_el.progress.style.width = width;

	if( p_playing != 0 )
	{
		p_StopTimer();
		p_timer = setTimeout('p_NextFrame()', 40);
	}

	p_SetPaintState();
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
	var width = p_el.slider.clientWidth - 1;
	var offset = i_evt.clientX - p_el.slider.offsetLeft;
	p_el.progress.style.width = 100.0*offset/width + '%';
	p_ShowFrame( Math.floor(p_elImg.length * offset / width ));
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

function p_Paint()
{
	if( p_painting )
	{
		p_el.paint.style.display = 'none';
		p_painting = false;
	}
	else
	{
		p_el.paint.style.display = 'block';
		p_painting = true;
	}
}

function p_ViewOnMouseDown( i_evt)
{
i_evt.stopPropagation();
	if( p_loaded == false ) return;
	if( p_painting == false ) return;

	var canvas = null;
	if( p_elCanvas[p_frame] )
	{
		canvas = p_elCanvas[p_frame];
	}
	else
	{
		canvas = document.createElement('canvas');
		p_elCanvas[p_frame] = canvas;
		p_el.view.appendChild( canvas);
		canvas.width = p_elImg[p_frame].width;
		canvas.height = p_elImg[p_frame].height;
		canvas.style.width = canvas.width + 'px';
		canvas.style.height = canvas.height + 'px';
		c_Info('Canvas for "'+p_files[p_frame]+'" created.');
	}

	var c = p_GetCtxCoords( i_evt);
	p_ctx = canvas.getContext('2d');
	p_ctx.globalCompositeOperation = 'source-over';
	p_ctx.beginPath();
	p_ctx.lineWidth = 10;
	p_ctx.lineCap = 'round';
	p_ctx.strokeStyle = 'rgb(255,255,0)';

	p_SetPaintState();
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
	return c;
}
function p_ViewOnMouseMove( i_evt)
{
	if( p_ctx == null ) return;
	var c = p_GetCtxCoords( i_evt);
	p_ctx.lineTo( c.x, c.y);
	var wasedited = p_elCanvas[p_frame].m_edited;
	p_elCanvas[p_frame].m_edited = true;
	p_elCanvas[p_frame].m_saved = false;
	p_ctx.stroke();
	p_SetPaintState();
}
function p_ViewOnMouseUp()
{
	p_ctx = null;
}
function p_ViewOnMouseOver()
{
	p_ctx = null;
}

function p_ColorChanged()
{
}

function p_Save()
{
	if( p_painting == false ) return;
	if( p_saving ) return;

	p_saving = true;
	p_filestosave = 0;
	p_filessaved = 0;

	p_el.btn_save.classList.add('pushed');
	for( var f = 0; f < p_files.length; f++)
	{
		var canvas = p_elCanvas[f];
		if( canvas == null ) continue;
		if( canvas.m_edited != true ) continue;
		if( canvas.m_saved ) continue;

		p_filestosave++;

		var ctx = canvas.getContext('2d');
		ctx.globalCompositeOperation = 'destination-over';
		ctx.drawImage( p_elImg[f], 0, 0);

		var data = canvas.toDataURL();
		var data = canvas.toDataURL('image/jpeg',.8);
		data = data.substr( data.indexOf(',')+1);

		var path = p_path.substr( 0, p_path.lastIndexOf('/'));
		var folder = p_path.substr( p_path.lastIndexOf('/')+1);
		path = path+'/'+folder+'.painted/'+p_files[f];

		n_Request({"save":path,"data":data}, false);
	}

	if( p_filestosave == 0 )
		p_SavingFinished();
}

function n_MessageReceived( i_msg)
{
//window.console.log(JSON.stringify(i_msg));
	var file = null;
	if( i_msg.save )
	{
		if( i_msg.save.error )
		{
			c_Error( i_msg.save.error);
			return;
		}
		file =  i_msg.save.file;
		if( file )
		{
			p_filessaved++;
			var name = file.substr( file.lastIndexOf('/')+1);
			var frame = p_files.indexOf(name);
			if( p_elCanvas[frame])
				p_elCanvas[frame].m_saved = true;
			p_SetPaintState();
			c_Info('Saved "'+name +'" '+p_filessaved+' of '+p_filestosave);
			if( p_filessaved >= p_filestosave )
				p_SavingFinished( file.substr( 0, file.lastIndexOf('/')).substr( file.indexOf('/')));
		}
	}
}

function p_SavingFinished( folder)
{
	p_saving = false;
	p_el.btn_save.classList.remove('pushed');
	if( folder )
		c_Info('Saved '+p_filessaved+' files to "'+folder+'"');
	else
		c_Info('Saved '+p_filessaved+' files');
}

function p_SetPaintState()
{
	var shadow = '0 0 4px #000';
	if( p_elCanvas[p_frame] )
	{
		shadow = '0 0 4px #FF0';
		if( p_elCanvas[p_frame].m_edited )
		{
			shadow = '0 0 4px #F00';
			if( p_elCanvas[p_frame].m_saved )
				shadow = '0 0 4px #0F0';
		}
	}
	p_el.view.style.boxShadow = shadow;
	p_el.btn_paint.style.boxShadow = shadow;
}

