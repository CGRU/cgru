Plotter_W = 50;
Plotter_H = 25;
Plotter_MW = 8;
Plotter_MH = 2;

function Plotter( i_plottersArray, i_pElement, i_label)
{
	this.elParent = i_pElement;
//	this.element = document.createElement('span');
	this.element = document.createElement('div');
	this.elParent.appendChild( this.element);
	this.element.classList.add('plotter');
	this.element.style.top = Plotter_MH + 'px';
	this.element.style.width = Plotter_W + 'px';
	this.element.style.height = Plotter_H + 'px';

	var dx = Plotter_W + Plotter_MW;
	this.element.style.left = (-3*dx + i_plottersArray.length * dx) + 'px';

    this.canvas = document.createElement('canvas');
	this.canvas.width = Plotter_W;
	this.canvas.height = Plotter_H;
	this.element.appendChild( this.canvas);

	if( this.canvas.getContext)
	{
		var ctx = this.canvas.getContext('2d');
		ctx.fillStyle = 'rgb(200,80,0)';
		ctx.fillRect( 0, 0, Plotter_W/2, Plotter_H/2);
		ctx.fillStyle = 'rgb(0,0,200)';
		ctx.fillRect( Plotter_W/2, Plotter_H/2, Plotter_W, Plotter_H);
	}


	this.elLabel = document.createElement('div');
	this.element.appendChild( this.elLabel);
	this.elLabel.classList.add('label');
	this.elLabel.innerHTML = i_label;

//	this.element.style.width = '50px';
//	this.element.style.height = '50px';
//	this.element.style.left = '50px';

	i_plottersArray.push( this);

	this.values = [];
	this.colors = [];
	this.colorsHot = [];
}

Plotter.prototype.setHidden = function( i_hide)
{
	if( i_hide )
		this.element.style.display = 'none';
	else
		this.element.style.display = 'block';
}

Plotter.prototype.addGraph = function( i_r, i_g, i_b, i_rHot, i_gHot, i_bHot)
{
	this.values.push([]);
	this.colors.push([ i_r, i_g, i_b]);
	if( i_rHot && i_gHot && i_bHot )
		this.colorsHot.push( i_rHot, i_gHot, i_bHot);
	else
		this.colorsHot.push( null);
}

Plotter.prototype.addValues = function( i_vals, i_store)
{
	if( i_vals.length != this.values.length )
	{
	}
}

