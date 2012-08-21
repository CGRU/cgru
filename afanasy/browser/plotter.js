Plotter_W = 50;
Plotter_H = 25;

function Plotter( i_plottersArray, i_pElement, i_label)
{
	this.elParent = i_pElement;
//	this.element = document.createElement('span');
	this.element = document.createElement('div');
	this.elParent.appendChild( this.element);
	this.element.classList.add('plotter');
	this.element.style.top = '2px';
	this.element.style.width = Plotter_W + 'px';
	this.element.style.height = Plotter_H + 'px';

	this.element.style.left = (-3*Plotter_W + i_plottersArray.length * Plotter_W) + 'px';

	this.elLabel = document.createElement('div');
	this.element.appendChild( this.elLabel);
	this.elLabel.classList.add('label');
	this.elLabel.innerHTML = i_label;

//	this.element.style.width = '50px';
//	this.element.style.height = '50px';
//	this.element.style.left = '50px';

	i_plottersArray.push( this);
}

Plotter.prototype.setHidden = function( i_hide)
{
	if( i_hide )
		this.element.style.display = 'none';
	else
		this.element.style.display = 'block';
}
