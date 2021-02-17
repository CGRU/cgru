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

	// List should be an objects not an array to store half_selected state,
	// when several statuses selected.
	// But when when it is not needed (on tasks, as several tasks can't be selected),
	// simple array can be passed.
	// So we shold construct an object from array here.
	if (Array.isArray(this.list))
	{
		let objects = {};
		for (let id of this.list)
			objects[id] = {'selected':true};
		this.list = objects;
	}
	else
	{
		// Prepare items list
		for (let id in this.list)
			if ( ! this.list[id].half_selected)
				this.list[id].selected = true;
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

	this.elAllShowBtn = document.createElement('div');
	this.elRoot.appendChild(this.elAllShowBtn);
	this.elAllShowBtn.classList.add('button');
	this.elAllShowBtn.textContent = this.label;
	this.elAllShowBtn.m_editlist = this;
	this.elAllShowBtn.onclick = function(e) {e.currentTarget.m_editlist.showAllItems();};
	this.elItems = document.createElement('div');
	this.elRoot.appendChild(this.elItems);

	this.elAddItem = document.createElement('div');
	this.elItems.appendChild(this.elAddItem);
	this.elAddItem.classList.add('editing');
	this.elAddItem.contentEditable = 'true';
	this.elAddItem.m_editlist = this;
	this.elAddItem.oninput = function(e){e.currentTarget.m_editlist.addInputProcess();}

	for (let id in this.list)
		this.appendItem(id, this.list[id]);
};

EditList.prototype.appendItem = function(i_id, i_item)
{
	let el = document.createElement('div');
	el.classList.add('tag');
	this.elItems.insertBefore(el, this.elAddItem);
	el.m_id = i_id;
	if (this.list_all[i_id] && this.list_all[i_id].title)
		el.textContent = this.list_all[i_id].title;
	else
		el.textContent = i_id;
	if (this.list_all[i_id] && this.list_all[i_id].tip)
		el.title = this.list_all[i_id].tip;

	if (this.name == 'flags')
	{
		el.classList.add('flag');
		if (RULES.flags[i_id])
			st_SetElColor({"color": RULES.flags[i_id].clr}, el);
	}
	if (this.name == 'artists')
	{
		el.classList.add('artist');
		if (i_id == g_auth_user.id)
			el.classList.add('me');
	}

	let icon = null;
	if (this.name == 'artists')
		icon = c_GetAvatar(i_id);
	if (icon)
	{
		el.classList.add('with_icon');
		el.style.backgroundImage = 'url(' + icon + ')';
	}

	if (this.list_all[i_id] && this.list_all[i_id].disabled)
		el.classList.add('disabled');

	if (i_item.half_selected)
		el.classList.add('half_selected');
	else
		el.classList.add('selected');

	// Delete button
	let elBtnDel = document.createElement('div');
	elBtnDel.classList.add('button','delete','right');
	elBtnDel.m_el = el;
	elBtnDel.m_editlist = this;
	elBtnDel.onclick = function(e){e.currentTarget.m_editlist.deleteItem(e.currentTarget.m_el);}
	el.appendChild(elBtnDel);

	this.addInputProcess();
}

EditList.prototype.deleteItem = function(i_el)
{
	i_el.parentNode.removeChild(i_el);
	this.list[i_el.m_id].selected = false;
	this.addInputProcess();
}

EditList.prototype.addInputProcess = function()
{
	let text = this.elAddItem.textContent.toLowerCase();
	//console.log(text);

	if (this.elMenu)
	{
		this.elRoot.removeChild(this.elMenu);
		this.elMenu.textContent = '';
		this.elMenu = null;
	}

	if (text.length < 1)
	{
		return;
	}

	if (null == this.elMenu)
	{
		this.elMenu = document.createElement('div');
		this.elRoot.appendChild(this.elMenu);
		this.elMenu.classList.add('menu');
	}

	let array = [];
	for (let id in this.list_all)
	{
		if (this.list[id] && this.list[id].selected)
			continue;

		if (this.list_all[id] && this.list_all[id].disabled)
			continue;

		let item = {};
		item.id = id;

		if (this.list_all[id] && this.list_all[id].title)
			item.title = this.list_all[id].title;
		else
			item.title = id;

		if (item.title.toLowerCase().indexOf(text) == -1)
			continue;

		array.push(item);
	}

	array.sort(function(a,b) {return a.title < b.title});

	let ids = []
	for (let i = 0; i < array.length; i++)
		ids.push(array[i].id);

	let elements = [];

	if (this.name == 'artists')
		elements = st_SetElArtists({'artists':ids}, this.elMenu);
	else if (this.name == 'tags')
		elements = st_SetElTags({'tags':ids}, this.elMenu);
	else if (this.name == 'flags')
		elements = st_SetElFlags({'flags':ids}, this.elMenu);
	else
		for (let i = 0; i < array.length; i++)
		{
			let el = document.createElement('span');
			el.classList.add('item');
			el.textContent = array[i].title;
			el.m_name = array[i].id;

			this.elMenu.appendChild(el);
			elements.push(el);
		}

	for (let i = 0; i < elements.length; i++)
	{
		let el = elements[i];
		el.classList.add('item');

		el.m_editlist = this;
		el.onclick = function(e){e.currentTarget.m_editlist.menuItemOnclick(e.currentTarget.m_name);};
	}
}

EditList.prototype.menuItemOnclick = function(i_id)
{
	let item = {'selected':true};
	this.list[i_id] = item;
	this.appendItem(i_id, item);
	this.elAddItem.textContent = '';
	this.elAddItem.focus();
}

EditList.prototype.showAllItems = function()
{
	if (this.m_showing_all)
		return;

	if (this.elMenu)
	{
		this.elRoot.removeChild(this.elMenu);
		this.elMenu.textContent = '';
		this.elMenu = null;
	}

	this.m_showing_all = true;
	this.elAllShowBtn.classList.remove('button');
	this.elItems.style.display = 'none';
	this.elRoot.classList.add('editing');

	this.elAllItems = [];

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
			if (this.list[item].half_selected)
			{
				el.m_half_selected = true;
				el.classList.add('half_selected');
			}
			else if(this.list[item].selected)
			{
				el.m_selected = true;
				el.classList.add('selected');
			}
		}

		el.onclick = editlist_ToggleSelection;

		this.elAllItems.push(el);
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
					if (this.list[artist.id].half_selected)
					{
						el.m_half_selected = true;
						el.classList.add('half_selected');
					}
					else if (this.list[artist.id].selected)
					{
						el.m_selected = true;
						el.classList.add('selected');
					}
				}

				el.onclick = editlist_ToggleSelection;

				this.elAllItems.push(el);
			}
		}
	}
};

EditList.prototype.getSelectedObjects = function()
{
	let objects = {};

	if (this.elAllItems)
	{
		for (let i = 0; i < this.elAllItems.length; i++)
		{
			if (this.elAllItems[i].m_selected)
				objects[this.elAllItems[i].m_item] = {'selected':true};
			else if (this.elAllItems[i].classList.contains('half_selected'))
				objects[this.elAllItems[i].m_item] = {'half_selected':true};
		}
	}
	else
	{
		for (let id in this.list)
			if (this.list[id].selected)
				objects[id] = {'selected':true};
			else if (this.list[id].half_selected)
				objects[id] = {'half_selected':true};
	}

	return objects;
}

EditList.prototype.getSelectedNames = function ()
{
	let names = [];

	if (this.elAllItems)
	{
		for (let i = 0; i < this.elAllItems.length; i++)
			if (this.elAllItems[i].m_selected)
				names.push(this.elAllItems[i].m_item);
	}
	else
	{
		for (let id in this.list)
			if (this.list[id].selected)
				names.push(id);
	}

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

