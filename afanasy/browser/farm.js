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

			let elName = document.createElement('div');
			elSrv.appendChild(elName);
			elName.textContent = srv;

			if (i_params.services_disabled && i_params.services_disabled.includes(srv))
			{
				elSrv.classList.add('disabled');
				elSrv.title = 'Service is disabled'
			}
			else
				elSrv.title = 'Service'
		}
	}
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

