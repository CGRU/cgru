function ad_OpenWindow()
{
	var wnd = new cgru_Window('Administate');

	var res = c_Parse( n_Request({"getusers":true}));
	if( res == null )
	{
		wnd.elContent.innerHTML = 'Error getting users.';
		return;
	}
	if( res.error )
	{
		wnd.elContent.innerHTML = 'Error getting users:<br>'+res.error;
		return;
	}

	wnd.elContent.classList.add('administrate');
	wnd.elContent.classList.add('admin_users');

	var labels = {};
	labels.del = 'DEL';
	labels.id = 'Name';
	labels.role = 'Role';
	labels.news = 'News';
	labels.ctime = 'Creation Time';
	labels.rtime = 'Refresh Time';
	ad_DrawUser( wnd.elContent, labels, true);

	for( var user in res.users )
	{
		ad_DrawUser( wnd.elContent, res.users[user]);
	}
}

function ad_DrawUser( i_el, i_user, i_labels)
{
		var el = document.createElement('div');
		i_el.appendChild(el);

		var elDel = document.createElement('div');
		el.appendChild( elDel);
		elDel.style.width = '40px';
		if( i_labels )
			elDel.innerHTML = i_user.del;
		else
			elDel.innerHTML = '-';

		var elName = document.createElement('div');
		el.appendChild( elName);
		if( i_labels )
			el.classList.add('labels');
		else
			el.classList.add('user');
		elName.style.width = '100px';
		elName.innerHTML = i_user.id;

		var elRole = document.createElement('div');
		el.appendChild( elRole);
		elRole.style.width = '100px';
		elRole.innerHTML = i_user.role;

		var elNews = document.createElement('div');
		el.appendChild( elNews);
		if( i_labels )
			elNews.innerHTML = i_user.news;
		else
			elNews.innerHTML = i_user.news.length;
		elNews.style.width = '50px';

		var elCTime = document.createElement('div');
		el.appendChild( elCTime);
		if( i_labels )
			elCTime.innerHTML = i_user.ctime;
		else
			elCTime.innerHTML = c_DT_StrFromS( i_user.ctime);
		elCTime.style.width = '200px';

		var elRTime = document.createElement('div');
		el.appendChild( elRTime);
		if( i_labels )
			elRTime.innerHTML = i_user.rtime;
		else
			elRTime.innerHTML = c_DT_StrFromS( i_user.rtime);
		elRTime.style.width = '200px';
}

