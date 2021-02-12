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
	editlist.js - Edit a list of items, such as tags, artists, flags.
*/

"use strict";

function EditList(i_args)
{
	this.name     = i_args.name;
	this.label    = i_args.label;
	this.list     = i_args.list;
	this.list_all = i_args.list_all;
	this.elParent = i_args.elParent;

	this.elItems  = null;

	// List should be an objects not an array to store half selected state,
	// when several statuses selected.
	// But when when it is not needed (on tasks, as several tasks can't be selected),
	// simple array can be passed.
	// So we shold construct an object from array here.
	if (Array.isArray(this.list))
	{
		let objects = {};
		for (let uid of this.list) objects[uid] = {};
		this.list = objects
	}

	this.elRoot = document.createElement('div');
	this.elParent.appendChild(this.elRoot);
	this.elRoot.classList.add('edit_tags_list');
	this.elRoot.classList.add(this.name);
	if (localStorage.background && localStorage.background.length)
		this.elRoot.style.background = localStorage.background;
	else
		this.elRoot.style.background = u_background;
	if (localStorage.text_color && localStorage.text_color.length)
		this.elRoot.style.color = localStorage.text_color;
	else
		this.elRoot.style.color = u_textColor;

	this.elBtn = document.createElement('div');
	this.elRoot.appendChild(this.elBtn);
	this.elBtn.classList.add('button');
	this.elBtn.textContent = this.label;
	this.elBtn.m_editlist = this;
	this.elBtn.onclick = function(e) {e.currentTarget.m_editlist.edit();};
	this.elList = document.createElement('div');
	this.elRoot.appendChild(this.elList);
	for (let id in this.list)
	{
		let el = document.createElement('div');
		el.classList.add('tag');
		this.elList.appendChild(el);
		if (this.list_all[id] && this.list_all[id].title)
			el.textContent = this.list_all[id].title;
		else
			el.textContent = id;
		if (this.list_all[id] && this.list_all[id].tip)
			el.title = this.list_all[id].tip;

		if (this.name == 'flags')
		{
			el.classList.add('flag');
			if (RULES.flags[id])
				st_SetElColor({"color": RULES.flags[id].clr}, el);
		}
		if (this.name == 'artists')
		{
			el.classList.add('artist');
			if (id == g_auth_user.id)
				el.classList.add('me');
		}

		let icon = null;
		if (this.name == 'artists')
			icon = c_GetAvatar(id);
		if (icon)
		{
			el.classList.add('with_icon');
			el.style.backgroundImage = 'url(' + icon + ')';
		}

		if (this.list_all[id] && this.list_all[id].disabled)
			el.classList.add('disabled');

		if (this.list[id].tooltip)
			el.title = this.list[id].tooltip;

		if (this.list[id].half)
			el.classList.add('half_selected');
		else
			el.classList.add('selected');
	}
};

EditList.prototype.edit = function()
{
	if (this.m_editing)
		return;

	this.m_editing = true;
	this.elBtn.classList.remove('button');
	this.elList.style.display = 'none';
	this.elRoot.classList.add('editing');

	this.elItems = [];

	if (this.name == 'artists')
	{
		this.editArtists();
		return;
	}

	let new_line = false;
	for (let item in this.list_all)
	{
		if (new_line)
		{
			let el = document.createElement('div');
			this.elRoot.appendChild(el);
			el.classList.add('new_line');
			new_line = false;
		}

		let el = document.createElement('div');
		this.elRoot.appendChild(el);
		el.classList.add('tag');
		if (this.name == 'flags')
		{
			el.classList.add('flag');
			if (RULES.flags[item] && RULES.flags[item].clr)
			{
				let c = RULES.flags[item].clr;
				el.style.borderColor = 'rgb(' + c[0]*0.5 + ',' + c[1]*0.5 + ',' + c[2]*0.5 + ')';
				el.style.backgroundColor = 'rgb(' + c[0] + ',' + c[1] + ',' + c[2] + ')';
			}

			if (RULES.flags[item] && RULES.flags[item].new_line)
				new_line = true;
		}

		el.m_item = item;

		if (this.list_all[item].title)
			el.textContent = this.list_all[item].title;
		else
			el.textContent = item;

		if (this.list_all[item].tip)
			el.title = this.list_all[item].tip;

		if (this.list[item])
		{
			if (this.list[item].half)
			{
				el.m_half_selected = true;
				el.classList.add('half_selected');
			}
			else
			{
				el.m_selected = true;
				el.classList.add('selected');
			}
		}

		el.onclick = editlist_ToggleSelection;

		this.elItems.push(el);
	}
};

EditList.prototype.editArtists = function()
{
	let roles = c_GetRolesArtists(this.list);

	for (let r = 0; r < roles.length; r++)
	{
		let elRole = document.createElement('div');
		this.elRoot.appendChild(elRole);
		elRole.classList.add('role');

		let elLabel = document.createElement('div');
		elRole.appendChild(elLabel);
		elLabel.classList.add('label');
		elLabel.textContent = roles[r].role + ':';

		for (let t = 0; t < roles[r].tags.length; t++)
		{
			let tag = roles[r].tags[t].tag;

			let elTag = document.createElement('div');
			elTag.classList.add('role_tag');
			elRole.appendChild(elTag);

			let elLabel = document.createElement('div');
			elLabel.textContent = c_GetTagTitle(tag) + ':';
			elLabel.title = c_GetTagTip(tag);
			elLabel.classList.add('label');
			elTag.appendChild(elLabel);

			for (let a = 0; a < roles[r].tags[t].artists.length; a++)
			{
				let artist = roles[r].tags[t].artists[a];

				let el = document.createElement('div');
				elTag.appendChild(el);
				el.classList.add('tag');
				el.classList.add('artist');
				el.m_item = artist.id;
				if (artist.id == g_auth_user.id)
					el.classList.add('me');

				if (g_users[artist.id] && g_users[artist.id].disabled)
					el.classList.add('disabled');

				if (artist.title)
					el.textContent = artist.title;
				else
					el.textContent = artist.id;

				if (artist.tip)
					el.title = artist.tip;

				let avatar = c_GetAvatar(artist.id);
				if (avatar)
				{
					el.classList.add('with_icon');
					el.style.backgroundImage = 'url(' + avatar + ')';
				}

				if (this.list[artist.id])
				{
					if (this.list[artist.id].half)
					{
						el.m_half_selected = true;
						el.classList.add('half_selected');
					}
					else
					{
						el.m_selected = true;
						el.classList.add('selected');
					}
				}

				el.onclick = editlist_ToggleSelection;

				this.elItems.push(el);
			}
		}
	}
};

EditList.prototype.getItems = function()
{
	return this.elItems;
}

EditList.prototype.getSelectedNames = function ()
{
	if ( ! this.elItems)
		return null;

	let names = [];
	for (let i = 0; i < this.elItems.length; i++)
		if (this.elItems[i].m_selected)
			names.push(this.elItems[i].m_item);

	return names;
}

function editlist_ToggleSelection(e)
{
	let el = e.currentTarget;
	if (el.m_selected)
	{
		el.m_selected = false;
		el.classList.remove('selected');
	}
	else if (el.classList.contains('half_selected'))
	{
		el.m_selected = true;
		el.classList.add('selected');
		el.classList.remove('half_selected');
	}
	else if (el.m_half_selected)
	{
		el.classList.add('half_selected');
	}
	else
	{
		el.m_selected = true;
		el.classList.add('selected');
	}
}

