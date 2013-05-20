function forum_InitTopic()
{
	a_SetLabel('Ask New Question');
	var data = n_Request({"getfile":'rules/assets/forum.html'});
	$('asset').innerHTML = data;

	u_GuestAttrsDraw( $('forum_guest_form'));

	if( g_auth_user == null ) $('forum_guest_form').style.display = 'block';
}

function forum_QuestionIDOnBlur( i_e )
{
	forum_QuestionIDOnGet();
}

function forum_QuestionIDOnGet()
{
	var id = $('forum_question_id').textContent;
	id = c_Strip( id.toLowerCase()).replace(/\W/g,'_').substr(0,16);
	$('forum_question_id').textContent = id;
	var walks = n_WalkDir([g_CurPath()]);
	if( walks && walks.length && walks[0].folders && walks[0].folders.length )
		for( var i = 0; i < walks[0].folders.length; i++)
			if( walks[0].folders[i].name == id )
			{
				c_Error('Question "' + id + '" already exists. Try other.');
				$('forum_question_id').classList.add('error');
				return null;
			}
	$('forum_question_id').classList.remove('error');
	return id;
}

function forum_NewQuestionOnClick()
{
	var question = {};

	question.id = forum_QuestionIDOnGet();
	if( question.id == null ) return;
	if( question.id.length == 0 )
	{
		c_Error('Required question ID attribute is empty.');
		$('forum_question_id').classList.add('error');
		return;
	}
	$('forum_question_id').classList.remove('error');

	question.body = c_Strip( $('forum_question_body').innerHTML);
	if( question.body.length == 0 )
	{
		c_Error('Question body is empty.');
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

	nw_MakeNews('<i>question</i>', folder, user_id, question.guest);
	g_GO( folder);
}

if( ASSETS.topic )
{
	if( ASSETS.topic.path == g_CurPath() ) forum_InitTopic();
	else $('asset_div').style.display = 'none';
}

