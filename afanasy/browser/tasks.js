function BlockItem() {}

BlockItem.prototype.init = function() 
{
	this.name = document.createElement('span');
	this.element.appendChild( this.name);
	this.name.title = 'Block name';
}

BlockItem.prototype.update = function()
{
	if( this.params.running === true )
	{
		if( false == this.element.classList.contains('running'))
		this.element.classList.add('running');
	}
	else
		this.element.classList.remove('running');

	this.name.innerHTML = this.params.name;
}

