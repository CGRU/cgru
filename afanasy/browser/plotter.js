Plotter_W = 54;
Plotter_H = 25;
Plotter_MW = 8;
Plotter_MH = 2;

Plotter_Lines = 100;
Plotter_MainW = 10;
Plotter_TailClrFade = 1;//.9;
Plotter_TailLineWidth = .8;
Plotter_AutoScaleLines = 30;
Plotter_AutoScaleMax = 1000000000;

function Plotter( i_plottersArray, i_pElement, i_label, i_title)
{
	this.label = i_label;
	this.title = i_title;

	this.elParent = i_pElement;
//	this.element = document.createElement('span');
	this.element = document.createElement('div');
	this.element.title = this.title;
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
	this.canvas.style.width = Plotter_W + 'px';
	this.canvas.style.height = Plotter_H + 'px';
	this.element.appendChild( this.canvas);

/*	if( this.canvas.getContext)
	{
		var ctx = this.canvas.getContext('2d');
		ctx.fillStyle = 'rgb(200,80,0)';
		ctx.fillRect( 0, 0, Plotter_W/2, Plotter_H/2);
		ctx.fillStyle = 'rgb(0,0,200)';
		ctx.fillRect( Plotter_W/2, Plotter_H/2, Plotter_W/2, Plotter_H/2);
	}
*/
	this.elLabel = document.createElement('div');
	this.element.appendChild( this.elLabel);
	this.elLabel.classList.add('label');
	this.elLabel.textContent = this.label;

//	this.element.style.width = '50px';
//	this.element.style.height = '50px';
//	this.element.style.left = '50px';

	i_plottersArray.push( this);

	this.pos = -1;
	this.values = [];
	this.colors = [];
	this.clrNrm = [];
	this.clrHot = [];
	this.scale = 100;
	this.cycle = 0;
	this.storeCycles = 1;
	this.clrBG = [0,0,0];
}

Plotter.prototype.setTitle = function( i_title)
{
	this.title = i_title;
	this.element.title = this.title;
}

Plotter.prototype.appendTitle = function( i_append)
{
	this.element.title = this.title + i_append;
}

Plotter.prototype.setHidden = function( i_hide)
{
	if( i_hide )
		this.element.style.display = 'none';
	else
		this.element.style.display = 'block';
}

Plotter.prototype.addGraph = function( i_storeCycles)
{
	this.values.push([]);
	this.colors.push([]);
	this.clrNrm.push([0,250,0]);
	this.clrHot.push( null);
	if( i_storeCycles )
		this.storeCycles = i_storeCycles;
}

Plotter.prototype.setColor = function( i_clr, i_clrHot)
{
	this.clrNrm[this.values.length-1] = i_clr;
	this.clrHot[this.values.length-1] = i_clrHot;
}

Plotter.prototype.setScale = function( i_scale, i_hot_min, i_hot_max)
{
	this.scale = i_scale;
	this.hot_min = i_hot_min;
	this.hot_max = i_hot_max;
}

Plotter.prototype.setAutoScale = function( i_label_value, i_maxBGC)
{
	this.scale = -1;
	this.autoScaleMaxBGC = i_maxBGC;
	this.label_value = i_label_value;
}


Plotter.prototype.addValues = function( i_vals, i_hot)
{
	if( i_vals.length != this.values.length )
		return;

	if( this.storeCycles > 1 )
	{
		if(( this.cycle % this.storeCycles ) == 0 )
		{
			this.pos++;
		}
	}
	else
		this.pos++;

	if( this.pos == Plotter_Lines )
		this.pos = 0;

	var clrs = [];
	for( var v = 0; v < i_vals.length; v++)
	{
		this.values[v][this.pos] = i_vals[v];

		clrs.push( [this.clrNrm[v][0], this.clrNrm[v][1], this.clrNrm[v][2]] );
		if( this.clrHot[v] && this.hot_min && this.hot_max )
		{
			if( i_hot )
			{
				for( var c = 0; c < 3; c++)
				{
					clrs[v][c] = Math.round( this.clrNrm[v][c] + i_hot * ( this.clrHot[v][c] - this.clrNrm[v][c] ));
				}
			}
			else if( i_vals[v] > this.hot_min )
			{
				for( var c = 0; c < 3; c++)
				{
					if( i_vals[v] >= this.hot_max )
					{
						clrs[v][c] = this.clrHot[v][c];
					}
					else
					{
						var val = ( i_vals[v] - this.hot_min ) / ( this.hot_max - this.hot_min );
						clrs[v][c] = Math.round( this.clrNrm[v][c] + val * ( this.clrHot[v][c] - this.clrNrm[v][c] ));
					}
				}
			}
		}

		this.colors[v][this.pos] = clrs[v];
		for( var c = 0; c < 3; c++)
			this.colors[v][this.pos][c] *= Plotter_TailClrFade;
	}

	this.cycle++;

	if( false == this.canvas.getContext )
		return;

	var scale = this.scale;
	if( scale <= 0 )
	{
		var max_value = 0;
		var p = this.pos;
		var line = 0;
		for( var i = 0; i < this.values[0].length; i++)
		{
			if( p == -1 )
				p = this.values[0].length - 1;
			var sum_values = 0;
			for( var v = 0; v < this.values.length; v++)
				sum_values += this.values[v][p];
			if( max_value < sum_values )
				max_value = sum_values;
			p--;
			line++;
			if( line >= Plotter_AutoScaleLines )
				break;
		}


		scale = 1;
		var scale_add = 1;
		var loop = 1;
		while( scale < Plotter_AutoScaleMax )
		{
			if( scale >= max_value) break;
			if( loop >= 10 )
			{
				scale_add *= 10;
				loop = 1;
			}
			scale += scale_add;
			loop ++;
		}
//g_Info( i_vals[0] + ' s'+scale + ' c'+(this.cycle));

		var val = scale;
		var blue = 128;
		while( val < this.autoScaleMaxBGC ) { val *= 10; blue /= 2;}
		if( blue <   0 ) blue = 0;
		if( blue > 255 ) blue = 255;
		this.clrBG[2] = blue;

		this.elLabel.textContent = this.label + ' ' + (scale/this.label_value);
	}

	var ctx = this.canvas.getContext('2d');
	ctx.fillStyle = 'rgb('+this.clrBG[0]+','+this.clrBG[1]+','+this.clrBG[2]+')';
	ctx.fillRect( 0, 0, Plotter_W, Plotter_H);

//g_Info( i_vals[0] + ' l'+this.values[1].length + ' s'+this.scale + ' c'+(cycle++));

	var x = Plotter_W - Plotter_MainW;
	var h0 = Plotter_H;
	for( var v = 0; v < this.values.length; v++)
	{
		ctx.fillStyle = 'rgb('+clrs[v][0]+','+clrs[v][1]+','+clrs[v][2]+')';
		var h = Plotter_H * this.values[v][this.pos] / scale;
		h0 -= h;
		ctx.fillRect( x, h0, Plotter_W, h);
	}

	if( this.values[0].length == 1 )
		return;

	ctx.lineWidth = Plotter_TailLineWidth;
	x -= 1;
	var p = this.pos-1;
	for( var i = 0; i < this.values[0].length-1; i++)
	{
		if( p == -1 )
			p = this.values[0].length - 1;
		h0 = Plotter_H;
		for( var v = 0; v < this.values.length; v++)
		{

			ctx.strokeStyle = 'rgb('+this.colors[v][p][0]+','+this.colors[v][p][1]+','+this.colors[v][p][2]+')';
			var h = h0 - (Plotter_H * this.values[v][p] / scale);
			ctx.beginPath();
			ctx.moveTo( x, h0);
			ctx.lineTo( x, h);
			ctx.stroke();
			h0 = h;
		}
		p--;
		x--;
		if( x == 0 )
			break;
	}

//	ctx.save();
}

