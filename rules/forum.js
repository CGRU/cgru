function forum_InitTopic()
{
	$('asset_label').textContent = 'Ask New Question'
	var data = n_Request({"getfile":'rules/forum.html'});
	$('asset').innerHTML = data;

	u_GuestAttrsDraw( $('forum_guest_form'));

	if( g_auth_user == null ) $('forum_guest_form').style.display = 'block';
}

function forum_NewQuestionOnClick()
{
	var question = {};
	question.id = c_Strip( $('forum_question_id').textContent).toLowerCase().replace(/\W/g,'_');
	question.body = c_Strip( $('forum_question_body').innerHTML);
	if( question.id.length == 0 )
	{
		c_Error('Required question ID attribute is empty.');
		return;
	}
	if( question.body.length == 0 )
	{
		c_Error('Required question body attribute is empty.');
		return;
	}

	var user_id = null;
	if( g_auth_user == null )
	{
		question.guest = u_GuestAttrsGet( $('forum_guest_form'));
		if( question.guest == null )
			return;
		user_id = question.guest.id;
	}
	else
		user_id = g_auth_user.id;

//console.log( JSON.stringify( question));

	var folder = ASSETS.topic.path + '/' + question.id;
	var path = RULES.root + '/' + folder  + '/' + RULES.rufolder + '/body.html';
	var result = c_Parse( n_Request({"save":{"file":path,"data":btoa(question.body),"type":"base64"}}));
	if( result.error )
	{
		c_Error( result.error);
		return;
	}

	var status = {};
	status.body = {};
	status.body.cuser = user_id;
	status.body.ctime = c_DT_CurSeconds();
	if( question.guest ) status.body.guest = question.guest;
	var result = st_Save( status, folder, true);
	if( result.error )
	{
		c_Error( result.error);
		return;
	}

//console.log( JSON.stringify( g_auth_user));
//console.log(user_id);

	nw_MakeNews('<i>question</i>', folder, user_id);
	g_GO( folder);
}

if( ASSETS.topic )
{
	if( ASSETS.topic.path == g_CurPath() ) forum_InitTopic();
	else $('asset_div').style.display = 'none';
}

