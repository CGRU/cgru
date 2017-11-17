/** '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                                      Copyright © 2016-17 by The CGRU team
 *    '          '
 * listen.js - window for monitoring (tail -f) all output of an entity
 * This is used for monitoring all output for a job, or inside the task to see the task output
 * ....................................................................................................... */

"use strict";

var listen_wnds = [];

var listen_Start = function(i_args) {
	var name = 'Listen Job';
	if (i_args.task != null)
		name = 'Listen Task';

	var wnd = new cgru_Window({"name": name, "wnd": i_args.parent_window});
	wnd.elContent.classList.add('listen');

	wnd.listen = i_args;
	wnd.processMsg = listen_ProcessMsg;
	wnd.onDestroy = listen_Subscribe;

	listen_Subscribe(wnd);
};

var listen_Subscribe = function(i_wnd) {
	var op = {};
	op.type = 'watch';
	op.class = 'listen';
	op.job = i_wnd.listen.job;
	op.status = i_wnd.subscribed ? 'unsubscribe' : 'subscribe';
	if (i_wnd.listen.task != null)
	{
		op.block = i_wnd.listen.block;
		op.task = i_wnd.listen.task;
	}

	if (i_wnd.subscribed)
	{
		g_ReceiverRemove(i_wnd);
		cm_ArrayRemove(listen_wnds, i_wnd);
	}
	else
	{
		g_ReceiverAdd(i_wnd);
		listen_wnds.push(i_wnd);
		i_wnd.subscribed = true;
	}

	nw_Action('monitors', [g_id], op);
};

var listen_ProcessMsg = function(i_obj) {
	if (i_obj.events == null)
		return;
	if (i_obj.events.tasks_listens == null)
		return;
	// console.log('listen_ProcessMsg: ' + JSON.stringify( i_obj))

	for (var l = 0; l < i_obj.events.tasks_listens.length; l++)
	{
		var listen = i_obj.events.tasks_listens[l];

		for (var w = 0; w < listen_wnds.length; w++)
		{
			var wListen = listen_wnds[w].listen;
			if ((listen.pos.job == wListen.job) &&
				((wListen.block == null) || (listen.pos.block == wListen.block)) &&
				((wListen.task == null) || (listen.pos.task == wListen.task)))
			{
				if (wListen.task == null)
				{
					var el = document.createElement('div');
					el.classList.add('name');
					el.textContent = listen.task_name + '[' + listen.progress.hst + ']:';
					listen_wnds[w].elContent.appendChild(el);
				}

				var el = document.createElement('pre');
				el.classList.add('output');
				el.textContent = listen.data;
				listen_wnds[w].elContent.appendChild(el);

				el.scrollIntoView(false);
			}
		}
	}
};
