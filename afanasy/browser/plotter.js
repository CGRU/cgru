Plotter_H = 25;

Plotter_MainW = 10;
Plotter_TailClrFade = 1;//.9;
Plotter_TailLineWidth = .8;
Plotter_AutoScaleLines = 30;
Plotter_AutoScaleMax = 1000000000;

function Plotter( i_pElement, i_label, i_title)
{
	this.label = i_label;
	this.title = i_title;

	this.elParent = i_pElement;
	this.element = document.createElement('div');
	this.element.title = this.title;
	this.elParent.appendChild( this.element);
	this.element.classList.add('plotter');

	this.canvas = document.createElement('canvas');
	this.element.appendChild( this.canvas);

	this.elLabel = document.createElement('div');
	this.element.appendChild( this.elLabel);
	this.elLabel.classList.add('label');
	this.elLabel.textContent = this.label;

	this.pos = -1;
	this.values = [];
	this.colors = [];
	this.clrNrm = [];
	this.clrHot = [];
	this.scale = 100;
	this.cycle = 0;
	this.storeCycles = 1;
	this.clrBG = [0,0,0];

	this.setHeight( Plotter_H);
}

Plotter.prototype.setWidth = function( i_width)
{
	this.width = i_width;
	this.element.style.width = this.width + 'px';
	this.canvas.width = this.width;
	this.canvas.style.width = this.width + 'px';
}
Plotter.prototype.setHeight = function( i_height)
{
	this.height = i_height;
	this.element.style.height = this.height + 'px';
	this.canvas.height = this.height;
	this.canvas.style.height = this.height + 'px';
}
Plotter.prototype.setLabel = function( i_label, i_clr, i_size)
{
	if( i_label )
		this.elLabel.innerHTML = i_label.replace(/\n/g,'<br>');
	if( i_clr )
		this.elLabel.color = 'rgb(' + i_clr[0] + ',' + i_clr[1] + ',' + i_clr[2] + ')';
	if( i_size )
		this.elLabel.style.fontSize = i_size + 'px';
}
Plotter.prototype.setBGColor = function( i_clr)
{
	this.clrBG = i_clr;
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
//if( this.element.classList.contains('custom')) console.log('Plotter.prototype.setScale:' + this.scale);
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
	{
		console.log('Graph count != values count');
		return;
	}

//if( this.element.classList.contains('custom')) console.log( i_vals);

	if( this.storeCycles > 1 )
	{
		if(( this.cycle % this.storeCycles ) == 0 )
		{
			this.pos++;
		}
	}
	else
		this.pos++;

	if( this.pos == this.width )
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
//if( this.element.classList.contains('custom')) console.log( scale);
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
	ctx.fillRect( 0, 0, this.width, this.height);

//g_Info( i_vals[0] + ' l'+this.values[1].length + ' s'+this.scale + ' c'+(cycle++));

	var x = this.width - Plotter_MainW;
	var h0 = this.height;
	for( var v = 0; v < this.values.length; v++)
	{
		ctx.fillStyle = 'rgb('+clrs[v][0]+','+clrs[v][1]+','+clrs[v][2]+')';
		var h = this.height * this.values[v][this.pos] / scale;
		h0 -= h;
		ctx.fillRect( x, h0, this.width, h);
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
		h0 = this.height;
		for( var v = 0; v < this.values.length; v++)
		{

			ctx.strokeStyle = 'rgb('+this.colors[v][p][0]+','+this.colors[v][p][1]+','+this.colors[v][p][2]+')';
//if( this.element.classList.contains('custom')) console.log( this.height + ' ' + this.values[v][p] + ' ' + scale);
			var h = h0 - (this.height * this.values[v][p] / scale);
			ctx.beginPath();
			ctx.moveTo( x, h0);
			ctx.lineTo( x, h);
			ctx.stroke();
//if( this.element.classList.contains('custom')) console.log(ctx.strokeStyle + ' ' + x + ' ' + h0 + ' ' + h);
			h0 = h;
		}
		p--;
		x--;
		if( x == 0 )
			break;
	}

//if( this.element.classList.contains('custom')) console.log( this.values[0].length);
//	ctx.save();
}

