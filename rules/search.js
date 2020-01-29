/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	search.js - TODO: description
*/

"use strict";

function s_SearchOnClick()
{
	// First time search operations:
	if ($('search').m_constcted != true)
	{
		$('search').m_constcted = true;
		$('search_artists').m_elRoles = [];
		$('search_artists').m_elArtists = [];
		$('search_artists').m_show_disabled = false;
		$('search_artists').m_show_not_artists = false;

		// Construct artists just once, as they are not depending on location:
		var roles = c_GetRolesArtists(g_users);

		for (let r = 0; r < roles.length; r++)
		{
			let elRole = document.createElement('div');
			$('search_artists').appendChild(elRole);
			$('search_artists').m_elRoles.push(elRole);
			elRole.classList.add('role');
			elRole.m_elTags = [];
			elRole.m_elArtists = [];

			let elLabel = document.createElement('div');
			elRole.appendChild(elLabel);
			elLabel.m_elRole = elRole;
			elLabel.classList.add('label');
			elLabel.textContent = roles[r].role + ':';
			elLabel.title = 'Click to (un)select all role artists.';
			elLabel.onclick = function(e) {
				let el = e.currentTarget.m_elRole;
				for (let a = 0; a < el.m_elArtists.length; a++)
					if (false == el.m_elArtists[a].m_hidden)
						c_ElToggleSelected(el.m_elArtists[a]);
				if (ASSET && ASSET.filter)
					s_ProcessGUI();
			};

			for (let t = 0; t < roles[r].tags.length; t++)
			{
				let tag = roles[r].tags[t].tag;

				let elTag = document.createElement('div');
				elTag.classList.add('role_tag');
				elRole.appendChild(elTag);
				elTag.m_elArtists = [];
				elRole.m_elTags.push(elTag);

				let elLabel = document.createElement('div');
				elLabel.textContent = c_GetTagTitle(tag) + ':';
				elLabel.classList.add('label');
				elTag.appendChild(elLabel);
				elLabel.title = c_GetTagTip(tag)
					+ '\nClick to (un)select all artists with "' + c_GetTagTitle(tag) + '" tag.';
				elLabel.m_elTag = elTag;
				elLabel.onclick = function(e) {
					let el = e.currentTarget.m_elTag;
					for (let a = 0; a < el.m_elArtists.length; a++)
						if (false == el.m_elArtists[a].m_hidden)
							c_ElToggleSelected(el.m_elArtists[a]);
					if (ASSET && ASSET.filter)
						s_ProcessGUI();
				};

				for (let a = 0; a < roles[r].tags[t].artists.length; a++)
				{
					let artist = roles[r].tags[t].artists[a];

					el = document.createElement('div');
					el.m_hidden = false;
					//elRole.appendChild(el);
					elTag.appendChild(el);
					elRole.m_elArtists.push(el);
					elTag.m_elArtists.push(el);
					$('search_artists').m_elArtists.push(el);
					el.style.cssFloat = 'left';
					el.textContent = c_GetUserTitle(artist.id);
					el.m_user = artist
					el.classList.add('tag');
					el.classList.add('artist');

					if (artist.id == g_auth_user.id)
						el.classList.add('me');

					if (artist.disabled)
						el.classList.add('disabled');

					if (c_IsNotAnArtist(artist))
						el.classList.add('notartist');

					var avatar = c_GetAvatar(artist.id);
					if (avatar)
					{
						el.classList.add('with_icon');
						el.style.backgroundImage = 'url(' + avatar + ')';
					}

					el.onclick = function(e) {
						c_ElToggleSelected(e);
						if (ASSET && ASSET.filter)
							s_ProcessGUI();
					};
				}
			}

			s_ShowHideRoles();
		}

		var el = $('search_artists_notassigned');
		$('search_artists').m_elArtists.push(el);
		el.m_user = {'id':'_null_'};
		el.classList.add('tag');
		el.classList.add('artist');
		el.onclick = function(e) {
			c_ElToggleSelected(e);
			if (ASSET && ASSET.filter)
				s_ProcessGUI();
		};
	}

	$('search_result').textContent = '';
	$('search_result_div').style.display = 'none';
	$('search_result_none').style.display = 'none';

	if ($('search_btn').m_opened)
	{
		$('search_btn').m_opened = false;
		$('search_btn').textContent = 'Search';
		$('search').style.display = 'none';
		$('search_btn_process').style.display = 'none';
		g_ClearLocationArgs();
		if (ASSET && window[ASSET.filter])
			s_Found(window[ASSET.filter]());
		$('search').m_path = null;
	}
	else
	{
		$('search_btn').m_opened = true;
		$('search_btn').textContent = 'Close';
		$('search').style.display = 'block';
		$('search_btn_process').style.display = 'block';

		// Flags:
		// Remove old:
		if ($('search_flags').m_elFlags)
			for (var i = 0; i < $('search_flags').m_elFlags.length; i++)
				$('search_flags').removeChild($('search_flags').m_elFlags[i]);
		$('search_flags').m_elFlags = [];

		// Create new:
		for (var flag in RULES.flags)
		{
			el = document.createElement('div');
			$('search_flags').appendChild(el);
			el.style.cssFloat = 'left';
			el.textContent = c_GetFlagTitle(flag);
			el.title = c_GetFlagTip(flag);
			el.m_flag = flag;
			el.classList.add('flag');
			if (RULES.flags[flag].clr)
			{
				var c = RULES.flags[flag].clr;
				el.style.borderColor = 'rgb(' + c[0] + ',' + c[1] + ',' + c[2] + ')';
			}
			el.onclick = function(e) {
				c_ElToggleSelected(e);
				if (ASSET && ASSET.filter)
					s_ProcessGUI();
			};
			$('search_flags').m_elFlags.push(el);
		}
		var el = $('search_noflags');
		$('search_flags').m_elFlags.push(el);
		el.m_flag = '_null_';
		el.onclick = function(e) {
			c_ElToggleSelected(e);
			if (ASSET && ASSET.filter)
				s_ProcessGUI();
		};

		// Tags:
		if ($('search_tags').m_elTags)
			for (var i = 0; i < $('search_tags').m_elTags.length; i++)
				$('search_tags').removeChild($('search_tags').m_elTags[i]);
		$('search_tags').m_elTags = [];
		for (var tag in RULES.tags)
		{
			el = document.createElement('div');
			$('search_tags').appendChild(el);
			el.style.cssFloat = 'left';
			el.textContent = c_GetTagTitle(tag);
			el.m_tag = tag;
			el.classList.add('tag');
			el.onclick = function(e) {
				c_ElToggleSelected(e);
				if (ASSET && ASSET.filter)
					s_ProcessGUI();
			};
			$('search_tags').m_elTags.push(el);
		}
		var el = $('search_notags');
		$('search_tags').m_elTags.push(el);
		el.m_tag = '_null_';
		el.onclick = function(e) {
			c_ElToggleSelected(e);
			if (ASSET && ASSET.filter)
				s_ProcessGUI();
		};

		if (ASSET && (ASSET.thumbnails != null))
			$('search_comment_div').style.display = 'none';
		else
			$('search_comment_div').style.display = 'block';
	}

	if (ASSET && window[ASSET.filter])
		s_Found(window[ASSET.filter]());
}

function s_ShowHideRoles()
{
	var show_disabled    = $('search_artists').m_show_disabled;
	var show_not_artists = $('search_artists').m_show_not_artists;
	var elRoles          = $('search_artists').m_elRoles;

	for (let r = 0; r < elRoles.length; r++)
	{
		let role_hidden = true;

		for (let t= 0; t < elRoles[r].m_elTags.length; t++)
		{
			let tag_hidden = true;

			for (let a = 0; a < elRoles[r].m_elTags[t].m_elArtists.length; a++)
			{

				let el = elRoles[r].m_elTags[t].m_elArtists[a];
				el.m_hidden = false;

				if (el.m_user.disabled && (false == show_disabled))
					el.m_hidden = true;

				if (c_IsNotAnArtist(el.m_user) && (false == show_not_artists))
					el.m_hidden = true;

				if (el.m_hidden)
					el.style.display = 'none';
				else
				{
					el.style.display = 'block';
					tag_hidden = false;
				}
			}

			if (tag_hidden)
				elRoles[r].m_elTags[t].style.display = 'none';
			else
			{
				elRoles[r].m_elTags[t].style.display = 'block';
				role_hidden = false;
			}
		}

		if (role_hidden)
			elRoles[r].style.display = 'none';
		else
			elRoles[r].style.display = 'block';
	}
}

function s_ShowDisabledArtists(i_el)
{
	i_el.classList.toggle('pushed');

	if (i_el.classList.contains('pushed'))
		$('search_artists').m_show_disabled = true;
	else
		$('search_artists').m_show_disabled = false;

	s_ShowHideRoles();
}

function s_ShowNotArtists(i_el)
{
	i_el.classList.toggle('pushed');

	if (i_el.classList.contains('pushed'))
		$('search_artists').m_show_not_artists = true;
	else
		$('search_artists').m_show_not_artists = false;

	s_ShowHideRoles();
}

function s_ProcessGUI()
{
	if ($('search').m_path && ($('search').m_path != g_CurPath()))
	{
		g_GO($('search').m_path);
		g_PathChanged();
	}
	$('search').m_path = g_CurPath();

	var args = {};

	// Depth:
	if (c_Strip($('search_depth').textContent).length)
		args.depth = parseInt(c_Strip($('search_depth').textContent));

	// Annotation:
	if (c_Strip($('search_annotation').textContent).length)
		args.ann = c_Strip($('search_annotation').textContent);

	// Artists:
	if ($('search_artists_notassigned').m_selected)
	{
		if (args.artists == null)
			args.artists = [];

		for (var i = 0; i < $('search_artists').m_elArtists.length; i++)
			c_ElSetSelected($('search_artists').m_elArtists[i], false);
		c_ElSetSelected($('search_artists_notassigned'), true);

		args.artists.push($('search_artists_notassigned').m_user.id);
	}
	else
		for (var i = 0; i < $('search_artists').m_elArtists.length; i++)
			if ($('search_artists').m_elArtists[i].m_selected)
			{
				if (args.artists == null)
					args.artists = [];
				if (false == $('search_artists').m_elArtists[i].m_hidden)
					args.artists.push($('search_artists').m_elArtists[i].m_user.id);
			}

	// Flags:
	if ($('search_noflags').m_selected)
	{
		if (args.flags == null)
			args.flags = [];
		for (var i = 0; i < $('search_flags').m_elFlags.length; i++)
			c_ElSetSelected($('search_flags').m_elFlags[i], false);
		c_ElSetSelected($('search_noflags'), true);

		args.flags.push($('search_noflags').m_flag);
	}
	else
		for (var i = 0; i < $('search_flags').m_elFlags.length; i++)
			if ($('search_flags').m_elFlags[i].m_selected)
			{
				if (args.flags == null)
					args.flags = [];
				args.flags.push($('search_flags').m_elFlags[i].m_flag);
			}

	// Tags:
	if ($('search_notags').m_selected)
	{
		if (args.tags == null)
			args.tags = [];
		for (var i = 0; i < $('search_tags').m_elTags.length; i++)
			c_ElSetSelected($('search_tags').m_elTags[i], false);
		c_ElSetSelected($('search_notags'), true);

		args.tags.push($('search_notags').m_tag);
	}
	else
		for (var i = 0; i < $('search_tags').m_elTags.length; i++)
			if ($('search_tags').m_elTags[i].m_selected)
			{
				if (args.tags == null)
					args.tags = [];
				args.tags.push($('search_tags').m_elTags[i].m_tag);
			}

	var parm = ['percent', 'finish', 'statmod', 'bodymod'];
	for (var i = 0; i < parm.length; i++)
	{
		var min = c_GetElInteger($('search_' + parm[i] + 'min'));
		var max = c_GetElInteger($('search_' + parm[i] + 'max'));
		if ((min != null) || (max != null))
			args[parm[i]] = [min, max];
	}

	if (c_Strip($('search_body').textContent).length)
		args.body = c_Strip($('search_body').textContent);
	if (c_Strip($('search_comment').textContent).length)
		args.comment = c_Strip($('search_comment').textContent);

	g_SetLocationArgs({"s_Search": args});
}

function s_Search(i_args)
{
	$('search_result').textContent = '';
	$('search_result_div').style.display = 'none';
	$('search_result_none').style.display = 'none';

	if ($('search_btn').m_opened !== true)
		s_SearchOnClick();

	if (i_args == null)
		i_args = {};

	if (i_args.depth)
		$('search_depth').textContent = i_args.depth;
	else
		i_args.depth = 1;

	var anns = null;
	if (i_args.ann)
		$('search_annotation').textContent = i_args.ann;
	if (i_args.artists)
		for (i = 0; i < $('search_artists').m_elArtists.length; i++)
			c_ElSetSelected(
				$('search_artists').m_elArtists[i],
				i_args.artists.indexOf($('search_artists').m_elArtists[i].m_user.id) != -1);
	if (i_args.flags)
		for (i = 0; i < $('search_flags').m_elFlags.length; i++)
			c_ElSetSelected(
				$('search_flags').m_elFlags[i],
				i_args.flags.indexOf($('search_flags').m_elFlags[i].m_flag) != -1);
	if (i_args.tags)
		for (i = 0; i < $('search_tags').m_elTags.length; i++)
			c_ElSetSelected(
				$('search_tags').m_elTags[i], i_args.tags.indexOf($('search_tags').m_elTags[i].m_tag) != -1)

				var parm = ['percent', 'finish', 'statmod', 'bodymod'];
	for (var i = 0; i < parm.length; i++)
		if (i_args[parm[i]])
		{
			if ((i_args[parm[i]][0] != null) && (i_args[parm[i]][1] != null) &&
				(i_args[parm[i]][0] > i_args[parm[i]][1]))
			{
				i_args[parm[i]][0] += i_args[parm[i]][1];
				i_args[parm[i]][1] = i_args[parm[i]][0] - i_args[parm[i]][1];
				i_args[parm[i]][0] = i_args[parm[i]][0] - i_args[parm[i]][1];
			}
			$('search_' + parm[i] + 'min').textContent = i_args[parm[i]][0];
			$('search_' + parm[i] + 'max').textContent = i_args[parm[i]][1];
		}

	if (i_args.body)
		$('search_body').textContent = i_args.body;
	if (i_args.comment)
		$('search_comment').textContent = i_args.comment;

	if (ASSET && ASSET.filter)
	{
		if (window[ASSET.filter])
			s_Found(window[ASSET.filter](i_args));
		return;
	}

	$('search_path').textContent = g_CurPath();
	$('search_path').href = '#' + g_CurPath();

	var args = {};
	for (var arg in i_args)
	{
		if (arg == 'comment')
		{
			args.comment = i_args['comment'];
			continue;
		}
		if (arg == 'body')
		{
			args.body = i_args['body'];
			continue;
		}
		if (arg == 'depth')
		{
			args.depth = i_args['depth'];
			continue;
		}
		if (args.status == null)
			args.status = {};
		args.status[arg] = i_args[arg];
	}
	args.path = RULES.root + g_CurPath();
	args.rufolder = RULES.rufolder;

	n_Request({"send": {"search": args}, "func": s_ResultReceived});
	$('search').classList.add('waiting');
}

function s_ResultReceived(i_data)
{
	$('search').classList.remove('waiting');

	var res = i_data;

	if (res.error)
	{
		c_Error(res.error);
		return;
	}

	if (res.result == null)
	{
		c_Error('Search returned null result.');
		return;
	}

	$('search_results_count').textContent = res.result.length;

	$('search_result_div').style.display = 'block';
	if (res.result.length == 0)
	{
		$('search_result').style.display = 'none';
		$('search_result_none').style.display = 'block';
		return;
	}
	$('search_result').style.display = 'block';

	res.result.sort();
	for (var i = 0; i < res.result.length; i++)
	{
		var path = res.result[i];
		path = path.replace(RULES.root, '');
		path = path.replace(/\/\//g, '/');
		var el = document.createElement('div');
		$('search_result').appendChild(el);
		var elLink = document.createElement('a');
		el.appendChild(elLink);
		elLink.href = '#' + path;
		elLink.textContent = path;
	}
}

function s_Found(i_args)
{
	if (i_args.found == null)
		return;

	var artists = i_args.found.artists;
	var   flags = i_args.found.flags;
	var    tags = i_args.found.tags;

	var elArtists = $('search_artists').m_elArtists;
	var elFlags   = $('search_flags').m_elFlags;
	var elTags    = $('search_tags').m_elTags;

	for (let e = 0; e < elArtists.length; e++)
	{
		let el = elArtists[e];
		let artist = el.m_user.id;
		if (artist == '_null_') continue;

		if (artists.indexOf(artist) == -1)
			el.classList.add('notfound');
		else
			el.classList.remove('notfound');
	}

	for (let e = 0; e < elFlags.length; e++)
	{
		let el = elFlags[e];
		let flag = el.m_flag;
		if (flag == '_null_') continue;

		if (flags.indexOf(flag) == -1)
			el.classList.add('notfound');
		else
			el.classList.remove('notfound');
	}

	for (let e = 0; e < elTags.length; e++)
	{
		let el = elTags[e];
		let tag = el.m_tag;
		if (tag == '_null_') continue;

		if (tags.indexOf(tag) == -1)
			el.classList.add('notfound');
		else
			el.classList.remove('notfound');
	}
}

