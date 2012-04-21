function RendersList()
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

RendersList.prototype.processMsg = function( obj)
{
//	document.getElementById('data').innerHTML=obj.renders;

	if( obj.renders == null )
		return;

	items = [];

	for( i = 0; i < obj.renders.length; i++)
	{
		items.push( new RenderNode(obj.renders[i]));
	}

	info('renders processed: ' + obj.renders.length);
}

function RenderNode( obj)
{
	this.element = document.createElement('p');
	document.getElementById('data').appendChild(this.element);
//	this.element.style.width = '50%';
	this.element.style.outlineStyle = 'solid';
	this.element.style.outlineWidth = '1px';
	this.element.style.outlineColor = '#777777';
	this.element.style.backgroundColor = '#EEEEEE';
	this.element.style.margin = '5px';
	this.element.style.padding = '5px';

	this.online = true;
	this.nimby  = false;
	this.NIMBY  = false;

	if( obj.offline === true ) this.online = false;
	if( obj.nimby   === true ) this.nimby  = true;
	if( obj.NIMBY   === true ) this.NIMBY  = true;

	this.name = document.createElement('span');
	this.element.appendChild(this.name);
	this.name.innerHTML = obj.name;
	this.name.style.backgroundColor = '#EEEE99';

	this.user_name = document.createElement('span');
	this.element.appendChild(this.user_name);
//	this.user_name.style.position = 'absolute';
//	this.user_name.style.top = '10px';
//	this.user_name.style.right = '10%';
//	this.user_name.style.width = '20%';
//	this.user_name.style.textAlign = 'right';
//	this.user_name.style.styleFloat = 'right';
	this.user_name.style.cssFloat = 'right';
//	this.user_name.style.margin = 'auto';
//	this.user_name.style.marginRight = '0px';
	this.user_name.innerHTML = obj.user_name;
	this.user_name.style.backgroundColor = '#EEEEBB';

	if( this.online )
	{
		if( this.nimby || this.NIMBY )
			this.element.style.backgroundColor = '#9999DD';
	}
	else
		this.element.style.backgroundColor = '#999999';
}

