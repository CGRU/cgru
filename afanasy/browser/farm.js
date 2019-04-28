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

