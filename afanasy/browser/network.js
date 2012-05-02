function send( obj)
{
	var obj_str = JSON.stringify(obj);

document.getElementById('send').innerHTML='c' + g_cycle + ' send: ' + obj_str;

	//document.getElementById("test").innerHTML='' + obj_str.length + ':' + obj_str;
	var xhr = new XMLHttpRequest;
/*	xhr.onerror = function()
	{
		document.getElementById("error").innerHTML='Error: ' + xhr.statusText();
	}*/
	xhr.open("POST", "/", true); 
	xhr.send('[ * AFANASY * ] 1 0 '+obj_str.length+' JSON'+obj_str);

	//document.getElementById("status").innerHTML='Status number = ' + xhr.status;
	//document.getElementById("statustext").innerHTML='Status text: ' + xhr.statusText;

	xhr.onreadystatechange = function()
	{
		if( xhr.readyState == 4 )
		{
			if( xhr.status == 200 )
			{
				processMsg( eval('('+xhr.responseText+')'));
			}
		}
	};
/*
	if(xhr.status == 200)
	{
		document.getElementById("type").innerHTML='Type = ' + xhr.responseType;
		document.getElementById("data").innerHTML=xhr.responseText;
	}
	document.getElementById("finish").innerHTML="It works!";
*/
}

function subscribe( i_class)
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
	obj.action.operation.status = "subscribe";

	send(obj);
}

function getEvents()
{
	if( g_id == 0 ) return;
//info('c' + g_cycle + ' getting events...');
	var obj = {};
	obj.get = {};
	obj.get.type = 'monitors';
	obj.get.ids = [g_id];
	obj.get.mode = 'events';

	send(obj);
}

function getNodes( i_type, i_ids)
{
	var obj = {};
	obj.get = {};
	obj.get.type = i_type;
	if(( i_ids != null ) && ( i_ids.length > 0 ))
		obj.get.ids = i_ids

	send(obj);
}
