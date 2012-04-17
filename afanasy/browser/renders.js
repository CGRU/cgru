function Renders()
{
	this.valid = false;
	this.name = 'renders';
	for( i = 0; i < recievers.lenght; i++)
	{
		if( recievers.name == this.name )
		{
			return;
		}
	}
	recievers.push( this);

	this.items = [];

	var obj = {};
	obj.get = {};
	obj.get.type = 'renders';
	send(obj);
}

Renders.prototype.processMsg = function( obj)
{
//	document.getElementById('data').innerHTML=obj.renders;

	if( obj.renders == null )
		return;

	items = [];

	for( i = 0; i < obj.renders.length; i++)
	{
		items.push( new Render(obj.renders[i]));
	}

	info('renders processed: ' + obj.renders.length);
}

function Render( obj)
{
	this.name = obj.name;
}

