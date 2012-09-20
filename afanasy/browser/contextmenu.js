function ContextMenu( i_elParent, i_evt, i_reciever, i_name)
{
	this.elParent = i_elParent;
	this.reciever = i_reciever;
	this.EType = 'ContextMenu';
	this.name = i_name;
	this.posX = i_evt.pageX;
	this.posY = i_evt.pageY;

	for( var i = 0; i < g_contextmenus.length; i++)
		if( g_contextmenus[i].name == this.name )
			g_contextmenus[i].destroy();

	this.element = document.createElement('div');
	this.elParent.appendChild( this.element);
	this.element.classList.add('contextmenu');

	g_contextmenus.push( this);
}

ContextMenu.prototype.destroy = function()
{
	cm_ArrayRemove( g_contextmenus, this);
	this.elParent.removeChild( this.element);
}

ContextMenu.prototype.show = function()
{
	this.element.style.display = 'block';

	var w = this.element.offsetWidth;
	var h = this.element.offsetHeight;

	if( this.posX + w > window.innerWidth  + window.pageXOffset) this.posX -= w;
	if( this.posY + h > window.innerHeight + window.pageYOffset) this.posY -= h;

	this.element.style.left = this.posX + 'px';
	this.element.style.top  = this.posY + 'px';
}

ContextMenu.prototype.addItem = function( i_name, i_actionName)
{
	var el = document.createElement('div');
	this.element.appendChild( el);
	el.innerHTML = i_name;
	el.parentmenu = this;
	el.actionName = i_actionName;
	el.onmousedown = this.onMouseUpDown;
	el.onmouseup   = this.onMouseUpDown;
}

ContextMenu.prototype.onMouseUpDown = function( evt)
{
	var el = evt.currentTarget;
	if( el == null ) return;
	if( el.parentmenu == null ) return;

	el.parentmenu.reciever.action( el.actionName);
	el.parentmenu.destroy();
}

