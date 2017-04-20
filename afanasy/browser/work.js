
function w_CreatePanels( i_monitor)
{
	var elPanelL = i_monitor.elPanelL;
	var elPanelR = i_monitor.elPanelR;

	var el = document.createElement('div');
	el.classList.add('section');
	elPanelR.appendChild( el);
	elPanelR.m_elPools = el;
	el.style.display = 'none';

	// Pools:
	var elPools = elPanelR.m_elPools;
	el = document.createElement('div');
	el.classList.add('caption');
	el.textContent = 'Pools:';
	elPools.appendChild( el);
	elPools.m_elArray = [];
}

function w_ResetPanels( i_monitor)
{
	var elPanelL = i_monitor.elPanelL;
	var elPanelR = i_monitor.elPanelR;

	// Pools:
	var elPools = elPanelR.m_elPools;
	for( var i = 0; i < elPools.m_elArray.length; i++)
		elPools.removeChild( elPools.m_elArray[i]);
	elPools.m_elArray = [];
	elPools.style.display = 'none';
}

function w_UpdatePanels( i_monitor, i_node)
{
	var elPanelL = i_monitor.elPanelL;
	var elPanelR = i_monitor.elPanelR;

	var work = i_node.params;

	// Pools:
	var elPools = elPanelR.m_elPools;
	var pools = work.pools;
	if( pools )
	{
		elPools.style.display = 'block';
		for( var pool in pools )
		{
			var el = document.createElement('div');
			elPools.appendChild( el);
			elPools.m_elArray.push( el);
			el.textContent = pool + ':' + pools[pool];

			el.m_node = i_node;
			el.m_pname = pool;
			el.m_pval = pools[pool];
			el.ondblclick = function(e){w_PoolDblClicked( e.currentTarget)}
		}
	}
}

function w_PoolDblClicked( i_el)
{
	var value = i_el.m_pname + ':' + i_el.m_pval;

	var args = {};

//	args.param    = 'pools';
	args.type     = 'str';
	args.receiver = this.window;
	args.wnd      = this.window;
	args.handle   = 'w_PoolSet';
	args.value    = value;
	args.name     = 'pool_parameter';
	args.info     = 'Edit pool:'

	new cgru_Dialog( args);
}

function w_PoolSet( i_value, i_param)
{
console.log('i_value = ' + i_value);
console.log('i_param = ' + i_param);
}

