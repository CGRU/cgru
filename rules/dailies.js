d_moviemaker = '/cgru/utilities/moviemaker';
d_makemovie = d_moviemaker+'/makemovie.py';

function d_Make( i_path, i_outfolder)
{
	c_Info('Make Dailies: '+i_path);

	version = i_path.split('/');
	version = version[version.length-1];

	artist = localStorage.user_title;
	activity = RULES.dailies.activity;

	var dateObj = new Date();
	date = ''+dateObj.getFullYear();
	date = date.substr(2);
	date += dateObj.getMonth() < 10 ? '0'+dateObj.getMonth() : dateObj.getMonth();
	date += dateObj.getDate()  < 10 ? '0'+dateObj.getDate()  : dateObj.getDate();

	var naming = RULES.dailies.naming;
	var outname = RULES.dailies.naming;
	outname = outname.replace('(p)', ASSETS.project.name);
	outname = outname.replace('(P)', ASSETS.project.name.toUpperCase());
	outname = outname.replace('(s)', ASSETS.shot.name);
	outname = outname.replace('(S)', ASSETS.shot.name.toUpperCase());
	outname = outname.replace('(v)', version);
	outname = outname.replace('(V)', version.toUpperCase());
	outname = outname.replace('(d)', date);
	outname = outname.replace('(D)', date.toUpperCase());
	outname = outname.replace('(a)', activity);
	outname = outname.replace('(A)', activity.toUpperCase());
	outname = outname.replace('(c)', RULES.company);
	outname = outname.replace('(C)', RULES.company.toUpperCase());
	outname = outname.replace('(u)', artist);
	outname = outname.replace('(U)', artist.toUpperCase());

	var wnd = new cgru_Window('dailes','Make Dailies');
/*
	var elArtistP = document.createElement('div');
	wnd.elContent.appendChild( elArtistP);
	var elArtistL = document.createElement('div');
	elArtistP.appendChild( elArtistL);
	elArtistL.textContent = 'Artist:';
	var elArtist = document.createElement('div');
	elArtistP.appendChild( elArtist);
*/
	wnd.elContent.classList.add('dailies');
	wnd.elContent.innerHTML ='\
	<div><div style="float:left">Artist:</div><div id="artist">'+artist+'</div></div>\
	<div><div style="float:left">Activity:</div><div id="activity">'+activity+'</div></div>\
	<div><div style="float:left">Output File Name:</div><div id="outname">'+outname+'</div></div>\
	<div><div>Rules:</div><div id="rules">'+JSON.stringify(RULES.dailies).replace(/,/g,', ')+'</div></div>\
	';

	var cmd = d_MakeCmd( i_path, i_outfolder+'/'+outname, version, artist, activity);
}

function d_MakeCmd( i_path, i_outfile, i_version, i_artist, i_activity)
{
	var cmd = 'python';

	cmd += ' "'+d_makemovie+'"';

	cmd += ' -c "'+RULES.dailies.codec+'"';
	cmd += ' -f '+RULES.dailies.fps;
	cmd += ' -r '+RULES.dailies.resolution;
	cmd += ' -s '+RULES.dailies.slate;
	cmd += ' -t '+RULES.dailies.template;
	cmd += ' --lgspath "'+RULES.dailies.logo_slate_path+'"';
	cmd += ' --lgssize '+RULES.dailies.logo_slate_size;
	cmd += ' --lgsgrav '+RULES.dailies.logo_slate_grav;
	cmd += ' --lgfpath "'+RULES.dailies.logo_frame_path+'"';
	cmd += ' --lgfsize '+RULES.dailies.logo_frame_size;
	cmd += ' --lgfgrav '+RULES.dailies.logo_frame_grav;

	cmd += ' --project "'+ASSETS.project.name+'"';
	cmd += ' --shot "'+ASSETS.shot.name+'"';

	cmd += ' --version "'+i_version+'"';
	cmd += ' --artist "'+i_artist+'"';
	cmd += ' --activity "'+i_activity+'"';

	cmd += ' "'+i_path+'"';
	cmd += ' "'+i_outfile+'"';

//python "/cgru/utilities/moviemaker/makemovie.py" -c "/cgru/utilities/moviemaker/codecs/photojpg_best.ffmpeg" -f 25 -n mov --fs 1 --fe 20 -r 720x576x1.09 -g 1.00 -s "dailies_slate" -t "dailies_withlogo" --project "ENCODE" --shot "preview" --ver "preview" --artist "Timurhai" --activity "comp" --tmpformat tga --lgspath "logo.png" --lgssize 25 --lgsgrav SouthEast --lgfpath "logo.png" --lgfsize 10 --lgfgrav North "/data/tools/encode/preview/preview.####.jpg" "/data/tools/encode/preview_preview_121226"

	c_Log(cmd);
	return cmd;
}

