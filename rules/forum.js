//$('asset').textContent = 'forum.js';

function initTopic()
{
//	$('asset').textContent = 'topic';
	$('asset').innerHTML = n_Get('rules/forum.html');
}

if( ASSETS.topic ) initTopic();

//console.log( JSON.stringify( ASSET))

