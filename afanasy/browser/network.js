nw_connected = false;
nw_send_count = 0;
nw_recv_count = 0;
nw_error_count = 0;
nw_error_count_max = 5;
nw_error_total = 0;

function nw_Send( obj)
{
	var obj_str = JSON.stringify(obj);

document.getElementById('send').innerHTML='c' + nw_send_count + ' send: ' + obj_str; nw_send_count++;

	var xhr = new XMLHttpRequest;
//	xhr.onerror = function() { g_Error(xhr.status + ':' + xhr.statusText); }
	xhr.open("POST", "/", true); 
	xhr.send('[ * AFANASY * ] 1 0 '+obj_str.length+' JSON'+obj_str);

	xhr.onreadystatechange = function()
	{
		if( xhr.readyState == 4 )
		{
			if( xhr.status == 200 )
			{
document.getElementById('recv').innerHTML='c' + nw_recv_count + ' recv: ' + xhr.responseText; nw_recv_count++;
				nw_error_count = 0;
				nw_connected = true;
				g_ProcessMsg( eval('('+xhr.responseText+')'));
				return;
			}
			nw_error_count++;
			nw_error_total++;
//document.getElementById("status").innerHTML=nw_error_count+': Status number = ' + xhr.status;
//document.getElementById("statustext").innerHTML='Status text: ' + xhr.statusText;
			if(( nw_error_count > nw_error_count_max ) && nw_connected )
			{
				nw_connected = false;
				g_Error('Connection lost.');
				g_ConnectionLost();
			}
		}
	}
}

function nw_Subscribe( i_class, i_subscribe, i_ids)
{
	if( g_id == 0 ) return;

	var obj = {};
	obj.action = {};
	obj.action.user_name = g_user_name;
	obj.action.host_name = g_host_name;
	obj.action.type = "monitors";
	obj.action.ids = [g_id];
	obj.action.operation = {};
	obj.action.operation.type = "watch";
	obj.action.operation.class = i_class;
	if( i_subscribe )
		obj.action.operation.status = "subscribe";
	else
		obj.action.operation.status = "unsubscribe";
	if( i_ids != null )
		obj.action.operation.ids = i_ids;
	obj.action.operation.uids = g_uids;

	nw_Send(obj);
}

function nw_GetEvents()
{
	if( g_id == 0 ) return;
//info('c' + g_cycle + ' getting events...');
	var obj = {};
	obj.get = {};
	obj.get.type = 'monitors';
	obj.get.ids = [g_id];
	obj.get.mode = 'events';

	nw_Send(obj);
}

function nw_GetNodes( i_type, i_ids, i_mode, i_blocks)
{
	var obj = {};
	obj.get = {};
	obj.get.type = i_type;
	if(( i_ids != null ) && ( i_ids.length > 0 ))
		obj.get.ids = i_ids;
	if( i_mode )
		obj.get.mode = i_mode;
	if( i_blocks )
		obj.get.block_ids = i_blocks;

	nw_Send(obj);
}

function nw_GetBlocks( i_job_id, i_blocks, i_modes)
{
	nw_GetNodes( 'jobs', [i_job_id], i_modes, i_blocks);
}

function nw_ReqestRendersResources()
{
	setTimeout("nw_ReqestRendersResources()", 4900);
	nw_GetNodes( 'renders');
}

