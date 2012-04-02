function rw_int32( i32)
{
	i1 = i32 * 256*256*256;
	return i1;
}

function init()
{
	document.getElementById("id").innerHTML="It works!";

	var myArray = new ArrayBuffer(512);
	var arrayView = new Int32Array(myArray);
	for ( var i = 0; i < arrayView.length; i++ )
	{ 
		arrayView[i] = 49;
	}
	arrayView[0] = rw_int32(44);
	arrayView[1] = rw_int32(1);
	arrayView[2] = rw_int32(0);
	arrayView[3] = rw_int32(4);
	arrayView[4] = rw_int32(1);
	var xhr = new XMLHttpRequest; 
	xhr.open("POST", "/", false); 
	xhr.send(myArray);

	document.getElementById("test").innerHTML="It works!";
}
