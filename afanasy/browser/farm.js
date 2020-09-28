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
	farm.js - farm related methods for Pools and Renderers
*/

"use strict";

function farm_showServices(i_el, i_params, i_type)
{
	// Show services
	if (i_params.services && i_params.services.length)
	{
		for (let srv of i_params.services)
		{
			let elSrv = document.createElement('div');
			i_el.appendChild(elSrv);
			elSrv.classList.add('service');

			if (cm_SoftwareIcons.includes(srv + '.png'))
			{
				let elIcon = document.createElement('img');
				elSrv.appendChild(elIcon);
				elIcon.src = 'icons/software/' + srv + '.png';
			}

			if (i_params.services_disabled && i_params.services_disabled.includes(srv))
			{
				elSrv.classList.add('disabled');
				elSrv.title = 'Service is disabled';
			}
			else
				elSrv.title = 'Service';

			// If serivce is runnint we show its count
			if (i_params.running_services)
			{
				for (let rs in i_params.running_services)
				{
					if (rs == srv)
					{
						elSrv.classList.add('running');
						srv += ': ' + i_params.running_services[rs];
						break;
					}
				}
			}

			let elName = document.createElement('div');
			elSrv.appendChild(elName);
			elName.textContent = srv;
		}
	}

	// Show running services that not exists in services (dummy):
	if (i_params.running_services && (i_type == 'pool'))
	{
		for (let srv in i_params.running_services)
		{
			if (i_params.services && i_params.services.includes(srv))
				continue;

			let elSrv = document.createElement('div');
			i_el.appendChild(elSrv);
			elSrv.classList.add('service');
			elSrv.classList.add('running');
			elSrv.classList.add('dummy');

			if (cm_SoftwareIcons.includes(srv + '.png'))
			{
				let elIcon = document.createElement('img');
				elSrv.appendChild(elIcon);
				elIcon.src = 'icons/software/' + srv + '.png';
			}

			let elName = document.createElement('div');
			elSrv.appendChild(elName);
			elName.textContent = srv + ': ' + i_params.running_services[srv];
		}
	}

	// Show disabled services
	if (i_params.services_disabled && i_params.services_disabled.length)
	{
		for (let srv of i_params.services_disabled)
		{
			if (i_params.services && i_params.services.includes(srv))
				continue;

			let elSrv = document.createElement('div');
			i_el.appendChild(elSrv);
			elSrv.classList.add('disabled_service');
			elSrv.textContent = srv;
			elSrv.title = 'Disabled service';
		}
	}
}

function farm_showServicesInfo(i_node)
{
	var elInfo = i_node.monitor.getElPanelInfo();
	var params = i_node.params;

	if (params.services && params.services.length)
	{
		let elServices = document.createElement('div');
		elInfo.appendChild(elServices);
		elServices.classList.add('info_services');

		let elServicesLabel = document.createElement('div');
		elServices.appendChild(elServicesLabel);
		elServicesLabel.classList.add('info_services_label');
		elServicesLabel.textContent = 'Services: ';

		for (let srv of params.services)
		{
			let title = '';

			let disabled = false;
			if (params.services_disabled && params.services_disabled.includes(srv))
			{
				title = 'Service "' + srv + '" is disabled.';
				disabled = true;
			}
			else
				title = 'Service "' + srv + '".';

			let elSrv = document.createElement('div');
			elServices.appendChild(elSrv);
			elSrv.classList.add('service');
			if (disabled)
				elSrv.classList.add('disabled');

			if (cm_SoftwareIcons.includes(srv + '.png'))
			{
				let elIcon = document.createElement('img');
				elSrv.appendChild(elIcon);
				elIcon.src = 'icons/software/' + srv + '.png';
				if (g_VISOR())
				{
					if (disabled)
					{
						elIcon.ondblclick = function(i_evt) {
							i_node.serviceApply(srv,'service_enable');
						}
						title += '\nDouble click icon to enable it.';
					}
					else
					{
						elIcon.ondblclick = function(i_evt) {
							i_node.serviceApply(srv,'service_disable');
						}
						title = '\nDouble click icon to disable it.';
					}
				}
			}

			if (g_VISOR())
			{
				let elBtn = document.createElement('div');
				elSrv.appendChild(elBtn);
				elBtn.classList.add('button');
				elBtn.classList.add('delete');
				elBtn.title = 'Double click to remove service.';
				elBtn.ondblclick = function(i_evt) {
					i_node.serviceApply(srv,'service_remove');
				}
			}

			let elName = document.createElement('div');
			elSrv.appendChild(elName);
			elName.classList.add('name');
			elName.textContent = srv;

			elSrv.title = title;
		}
	}

	if (params.services_disabled && params.services_disabled.length)
	{
		let elDisabled = null;

		for (let srv of params.services_disabled)
		{
			if (params.services && params.services.includes(srv))
				continue;

			// We should create disabled services control
			// only if there is at least one disabled serivce,
			// that is not in services (not shown above).
			if (elDisabled == null)
			{
				elDisabled = document.createElement('div');
				elInfo.appendChild(elDisabled);
				elDisabled.classList.add('info_services');

				let elDisabledLabel = document.createElement('div');
				elDisabled.appendChild(elDisabledLabel);
				elDisabledLabel.classList.add('info_services_label');
				elDisabledLabel.textContent = 'Disabled services: ';
			}

			let title = 'Disabled service "' + srv + '"';

			let elSrv = document.createElement('div');
			elDisabled.appendChild(elSrv);
			elSrv.classList.add('disabled_service');

			if (g_VISOR())
			{
				let elBtn = document.createElement('div');
				elSrv.appendChild(elBtn);
				elBtn.classList.add('button');
				elBtn.classList.add('delete');
				elBtn.title = 'Double click to remove service.';
				elBtn.ondblclick = function(i_evt) {
					i_node.serviceApply(srv,'service_enable');
				}
			}

			let elName = document.createElement('div');
			elSrv.appendChild(elName);
			elName.classList.add('name');
			elName.textContent = srv;

			elSrv.title = title;
		}
	}
}

function farm_showTickets(i_el, i_tickets, i_type, i_node)
{
	if (i_tickets == null)
		return;

	for (let tk in i_tickets)
	{
		let count = i_tickets[tk][0];
		let usage = i_tickets[tk][1];
		let hosts = i_tickets[tk][2];
		let max_hosts = i_tickets[tk][3];
		let dummy = false;

		if ((i_node.node_type == 'renders') && (count == -1) && (i_node.m_parent_pool))
		{
			count = i_node.m_parent_pool.getTicketHostCount(tk);
			if (count == -1)
				continue;

			dummy = true;
		}

		let elTk = document.createElement('div');
		i_el.appendChild(elTk);
		elTk.classList.add('service');
		elTk.classList.add('ticket');
		elTk.classList.add(i_type);
		if ((count < 0) || dummy)
			elTk.classList.add('dummy');
		if (usage > 0)
			elTk.classList.add('running');
		if (usage >= count)
			elTk.classList.add('limit_reached')
		if ((max_hosts != null) && (max_hosts != -1) && (hosts >= max_hosts))
			elTk.classList.add('limit_reached');

		let label = '';
		if (cm_TicketsIcons.includes(tk + '.png'))
		{
			let elIcon = document.createElement('img');
			elTk.appendChild(elIcon);
			elIcon.src = 'icons/tickets/' + tk + '.png';
		}
		else
			label = tk;

		if (count >= 0) label += ' x' + count;
		if (usage >  0) label += ': ' + usage;
		if (hosts     ) label += '/'  + hosts;
		if ((max_hosts != null) && (max_hosts != -1))
			label += '/'  + max_hosts;

		let elLabel = document.createElement('div');
		elTk.appendChild(elLabel);
		elLabel.textContent = label;
	}
}

function farm_showTicketsInfo(i_node, i_type)
{
	var tickets = i_node.params['tickets_' + i_type];
	if (tickets == null)
		return;

	let elTickets = document.createElement('div');
	i_node.monitor.getElPanelInfo().appendChild(elTickets);
	elTickets.classList.add('info_services');
	elTickets.classList.add('info_tickets');

	let elTicketsLabel = document.createElement('div');
	elTickets.appendChild(elTicketsLabel);
	elTicketsLabel.classList.add('info_services_label');
	elTicketsLabel.textContent = 'Tickets ' + i_type + ':';

	for (let tk in tickets)
	{
		let count = tickets[tk][0];
		let usage = tickets[tk][1];
		let hosts = tickets[tk][2];
		let max_hosts = tickets[tk][3];

		let elTk = document.createElement('div');
		elTickets.appendChild(elTk);
		elTk.classList.add('service');
		elTk.classList.add('ticket');
		elTk.classList.add(i_type);
		if (count < 0)
			elTk.classList.add('dummy');
		if (usage > 0)
			elTk.classList.add('running');
		if (usage >= count)
			elTk.classList.add('limit_reached');
		if ((max_hosts != null) && (max_hosts != -1) && (hosts >= max_hosts))
			elTk.classList.add('limit_reached');

		if (cm_TicketsIcons.includes(tk + '.png'))
		{
			let elIcon = document.createElement('img');
			elTk.appendChild(elIcon);
			elIcon.src = ('icons/tickets/' + tk + '.png');
		}

		let label = tk + ':';
		if (count !=-1) label += ' count:' + count;
		if (usage  > 0) label += ' usage:' + usage;
		if (hosts     ) label += ' hosts:' + hosts;
		if ((max_hosts != null) && (max_hosts != -1))
			label += ' max:' + max_hosts;

		let elLabel = document.createElement('div');
		elTk.appendChild(elLabel);
		elLabel.classList.add('name');
		elLabel.textContent = label;

		elTk.m_name = tk;
		elTk.m_count = count;
		elTk.m_max_hosts = max_hosts;
		elTk.m_type = i_type;
		elTk.m_node = i_node;
		elTk.ondblclick = function(e){
			var el = e.currentTarget;
			farm_ticketEditDialog(el.m_name, el.m_count, el.m_max_hosts, el.m_type, el.m_node);
		}
	}
}

function farm_ticketEditDialog(i_name, i_count, i_max_hosts, i_type, i_node)
{
	var args = {};

	if ((i_name == null) || (i_name.length == 0))
	{
		g_Error('Invalid ticket name');
		return;
	}

	args.param = {};
	args.param.name = i_name;
	args.param.type = i_type;
	args.param.node = i_node;
	args.param.max_hosts = i_max_hosts;

	args.type = 'num';
	args.receiver = i_node.monitor.window;
	args.handle = 'farm_ticketEditApply';
	// On 'pool' thicket we should ask for max hosts
	if (i_type == 'pool')
		args.handle = 'farm_ticketEditDialog2';
	args.value = i_count;
	args.name = 'ticket_edit';
	args.title = 'Edit Ticket';
	args.info = 'Enter  "' + i_name + '" ' + i_type + ' ticket count:<br>Type -1 to remove ticket.';

	new cgru_Dialog(args);
}

function farm_ticketEditDialog2(i_value, i_param)
{
	i_param.count = i_value;

	var args = {};
	args.param = i_param;
	args.type = 'num';
	args.receiver = i_param.node.monitor.window;
	args.handle = 'farm_ticketEditApply';
	args.value = i_param.max_hosts;
	// If not set, limit is disabled (value == -1)
	if (args.value == null)
		args.value = -1;
	args.name = 'ticket_edit';
	args.title = 'Edit Ticket';
	args.info = 'Enter  "' + i_param.name + '" ' + i_param.type + ' ticket max hosts:<br>Type -1 to disable limit.';

	new cgru_Dialog(args);
}

function farm_ticketEditApply(i_value, i_param)
{
	var operation = {};
	operation.type = 'tickets';
	operation.name = i_param.name;
	operation.count = i_value;

	if (i_param.count != null)
	{
		// If count is stored, we here from max hosts dialog (farm_ticketEditDialog2)
		operation.count = i_param.count;
		operation.max_hosts = i_value;
	}

	if (i_param.type == 'host')
		operation.host = true;

	nw_Action(i_param.node.node_type, [i_param.node.params.id], operation);
}
