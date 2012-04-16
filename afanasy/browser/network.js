function msgReceived( obj)
{
	for( i = 0; i < recievers.length; i++)
	{
		recievers[i].processMsg( obj);
	}
}

function send( obj)
{
	obj_str = JSON.stringify(obj);

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
				msgReceived( eval('('+xhr.responseText+')'));
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
