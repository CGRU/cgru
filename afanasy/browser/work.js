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
	work.js - work related methods for Branches, Jobs, Users
*/

"use strict";

var work_params = {
	priority /***************/: {'type': 'num', 'label': 'Priority'},
	pools                     : {"type": 'msi', "label": 'Pools'},
	max_running_tasks /******/: {'type': 'num', 'label': 'Max Running Tasks'},
	max_running_tasks_per_host: {'type': 'num', 'label': 'Max Running Tasks Per Host'},
	hosts_mask /*************/: {'type': 'reg', 'label': 'Hosts Mask'},
	hosts_mask_exclude /*****/: {'type': 'reg', 'label': 'Exclude Hosts Mask'},
	max_tasks_per_second /***/: {'type': 'num', 'label': 'Max Tasks Per Second'},
	annotation /*************/: {'type': 'str', 'label': 'Annotation'}
};

function work_generateParamsString(i_params, i_type)
{
	var str = '';

	if (i_type == null)
		i_type = 'branch';

	if (cm_IsPadawan())
	{
		if ((i_params.max_running_tasks != null) && (i_params.max_running_tasks != -1))
			str += " MaxRunTasks:<b>" + i_params.max_running_tasks + "</b>";
		if ((i_params.max_running_tasks_per_host != null) && (i_params.max_running_tasks_per_host != -1))
			str += " MaxPerHost:<b>" + i_params.max_running_tasks_per_host + "</b>";
		if (i_params.hosts_mask)
			str += " HostsMask:<b>" + i_params.hosts_mask + "</b>";
		if (i_params.hosts_mask_exclude)
			str += " ExcludeHosts:<b>" + i_params.hosts_mask + "</b>";
		if (i_type != 'job')
		{
			str += " Solving:";
			str += (i_params.solve_method == 'solve_priority') ? ' <b>Priority</b>' : ' <b>Order</b>';
			str += (i_params.solve_need   == 'solve_capacity') ? ', <b>Capacity</b>' : ', <b>RunTasks</b>';
		}
		if (i_params.max_tasks_per_second != null)
		{
			if (i_params.max_tasks_per_second == 0)
				str += ' <b style="color:darkred">MTPS:' + i_params.max_tasks_per_second + '</b>';
			else
				str += ' MTPS:<b>' + i_params.max_tasks_per_second + '</b>';
		}

		str += " Priority:<b>" + i_params.priority + "</b>";
	}
	else if (cm_IsJedi())
	{
		if ((i_params.max_running_tasks != null) && (i_params.max_running_tasks != -1))
			str += " Max:<b>" + i_params.max_running_tasks + "</b>";
		if ((i_params.max_running_tasks_per_host != null) && (i_params.max_running_tasks_per_host != -1))
			str += " MPH:<b>" + i_params.max_running_tasks_per_host + "</b>";
		if (i_params.hosts_mask)
			str += " Hosts:<b>" + i_params.hosts_mask + "</b>";
		if (i_params.hosts_mask_exclude)
			str += " Exclude:<b>" + i_params.hosts_mask + "</b>";
		if (i_type != 'job')
		{
			str += " Slv:";
			str += (i_params.solve_method == 'solve_priority') ? '<b>Priority</b>' : '<b>Order</b>';
			str += (i_params.solve_need   == 'solve_capacity') ? ',<b>Capacity</b>' : ',<b>RunTasks</b>';
		}
		if (i_params.max_tasks_per_second != null)
		{
			if (i_params.max_tasks_per_second == 0)
				str += ' <b style="color:darkred">MTPS:' + i_params.max_tasks_per_second + '</b>';
			else
				str += ' MTPS:<b>' + i_params.max_tasks_per_second + '</b>';
		}
		str += " Pri:<b>" + i_params.priority + "</b>";
	}
	else
	{
		if ((i_params.max_running_tasks != null) && (i_params.max_running_tasks != -1))
			str += " m:<b>" + i_params.max_running_tasks + "</b>";
		if ((i_params.max_running_tasks_per_host != null) && (i_params.max_running_tasks_per_host != -1))
			str += " mph:<b>" + i_params.max_running_tasks_per_host + "</b>";
		if (i_params.hosts_mask)
			str += " h:<b>" + i_params.hosts_mask + "</b>";
		if (i_params.hosts_mask_exclude)
			str += " e:<b>" + i_params.hosts_mask + "</b>";
		if (i_type != 'job')
		{
			str += " s:";
			str += (i_params.solve_method == 'solve_priority') ? '<b>pri</b>' : '<b>ord</b>';
			str += (i_params.solve_need   == 'solve_capacity') ? ',<b>cap</b>' : ',<b>tasts</b>';
		}
		if (i_params.max_tasks_per_second != null)
		{
			if (i_params.max_tasks_per_second == 0)
				str += ' <b style="color:darkred">MTPS:' + i_params.max_tasks_per_second + '</b>';
			else
				str += ' MTPS:<b>' + i_params.max_tasks_per_second + '</b>';
		}
		str += " <b>" + i_params.priority + "</b>";
	}

	return str;
}

function work_generateRunningCountsString(i_params, i_type)
{
	var str = '';

	if (i_type == null)
		i_type = 'branch';

	if (cm_IsPadawan())
	{
		if (i_params.running_tasks_num)
			str += "Running Tasks:<b>" + i_params.running_tasks_num + "</b>";
		if (i_params.running_capacity_total)
			str += " Capacity:<b>" + cm_ToKMG(i_params.running_capacity_total) + "</b>";
	}
	else if (cm_IsJedi())
	{
		if (i_params.running_tasks_num)
			str += "Tasks:<b>" + i_params.running_tasks_num + "</b>";
		if (i_params.running_capacity_total)
			str += " Capacity:<b>" + cm_ToKMG(i_params.running_capacity_total) + "</b>";
	}
	else
	{
		if (i_params.running_tasks_num)
			str += "t:<b>" + i_params.running_tasks_num + "</b>";
		if (i_params.running_capacity_total)
			str += " c:<b>" + cm_ToKMG(i_params.running_capacity_total) + "</b>";
	}

	return str;
}

function work_CreatePanels(i_monitor, i_type)
{
	var elPanelL = i_monitor.elPanelL;
	var elPanelR = i_monitor.elPanelR;


	// Work parameters below are not available for jobs
	if (i_type == 'jobs')
		return;


	// Solving:
	var acts = {};
	acts.solve_ord = {
		'name': 'solve_method',
		'value': 'solve_order',
		'label': 'ORDER',
		'tooltip': 'Solve jobs by order.',
		'handle': 'mh_Param'
	};
	acts.solve_pri = {
		'name': 'solve_method',
		'value': 'solve_priority',
		'label': 'PRIORITY',
		'tooltip': 'Solve jobs by priority.',
		'handle': 'mh_Param'
	};
	acts.solve_cap = {
		'name': 'solve_need',
		'value': 'solve_capacity',
		'label': 'CAPACITY',
		'tooltip': 'Solve need by running capacity total.',
		'handle': 'mh_Param'
	};
	acts.solve_tsk = {
		'name': 'solve_need',
		'value': 'solve_tasksnum',
		'label': 'TASKS',
		'tooltip': 'Solve need by running tasks number.',
		'handle': 'mh_Param'
	};
	i_monitor.createCtrlBtns(acts);
}
